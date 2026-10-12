#define RLIGHTS_IMPLEMENTATION
#define RAYGUI_IMPLEMENTATION

#include <numbers>
#include <cmath> 
#include <vector>
#include <memory>
#include <array>
#include <optional>
#include <functional>
#include <algorithm>
#include <tuple>
#include <fstream>
#include <iomanip>

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "rlights.h"
#include "vecutils.h"
#include "vec3.h"
#include "scene.h"
#include "raygui.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
    #include <emscripten/bind.h>
    #include <emscripten/val.h>
#endif

int GLOBAL_SCALE_2D = 200;
float LOCAL_EPSILON = 0.01f;

void drawGridByCurrentCamera(Camera2D& camera){
    int slices = 100;
    int dimension = 10;
    //наименьшее и наибольшее значение для координат
    int minPoint = -slices * dimension * GLOBAL_SCALE_2D / 2;
    int maxPoint = slices * dimension * GLOBAL_SCALE_2D / 2;
    BeginMode2D(camera);
    //сетка
    for(int point = minPoint; point <= maxPoint; point += dimension *  GLOBAL_SCALE_2D){
        DrawLine(point, minPoint, point, maxPoint, LIGHTGRAY);
        DrawLine(minPoint, point, maxPoint, point, LIGHTGRAY);
    }
    //Ось X
    DrawLine(minPoint, 0, maxPoint, 0, GREEN);
    //Ось Y
    DrawLine(0, minPoint, 0, maxPoint, GREEN);
    EndMode2D();
}

void drawMouseCoords(const Camera2D& camera){
    Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    DrawTextEx(GetFontDefault(), TextFormat("[%.1f, %.1f]", mouseWorldPos.x / GLOBAL_SCALE_2D, -mouseWorldPos.y / GLOBAL_SCALE_2D),
                Vector2Add(GetMousePosition(), (Vector2){ -44, -24 }), 20, 2, BLACK);
}

void updateCameraByMouseControl(Camera2D& camera){
    int zoomMode = 0; 
     if (IsKeyPressed(KEY_ONE)) zoomMode = 0;
    else if (IsKeyPressed(KEY_TWO)) zoomMode = 1;

    
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
    {
        Vector2 delta = GetMouseDelta();
        delta = Vector2Scale(delta, -1.0f/camera.zoom);
        camera.target = Vector2Add(camera.target, delta);
    }
    
    if (zoomMode == 0)
    {
        float wheel = GetMouseWheelMove();
            if (wheel != 0)
            {
                Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
                camera.offset = GetMousePosition();
                camera.target = mouseWorldPos;
                float scale = 0.2f*wheel;
                camera.zoom = Clamp(expf(logf(camera.zoom)+scale), 0.01f, 64.0f);
            }
    }
}

void renderScene(const Camera2D& camera, const Scene& scene){
    BeginMode2D(camera);
    for(int i = 0; i < scene.items.size(); i++){
        SceneFigure* figure = scene.items[i].get();
        if(figure->type == FigureTypes::Line && !figure->hidden){
            auto line = static_cast<FigureLine*>(figure);
            DrawLine(
                line->first.x * GLOBAL_SCALE_2D,
                -line->first.y * GLOBAL_SCALE_2D,
                line->second.x * GLOBAL_SCALE_2D,
                -line->second.y * GLOBAL_SCALE_2D, 
                RED
            );

        } else if(figure->type == FigureTypes::Circle && !figure->hidden){
            auto circle = static_cast<FigureCircle*>(figure);
            DrawCircleLines(
                circle->center.x * GLOBAL_SCALE_2D, 
                -circle->center.y * GLOBAL_SCALE_2D, 
                circle->radius * GLOBAL_SCALE_2D, 
                RED
            );
        } else if(figure->type == FigureTypes::Point && !figure->hidden){
            auto point = static_cast<FigurePoint*>(figure);
            DrawCircle(
                point->pos.x * GLOBAL_SCALE_2D, 
                -point->pos.y * GLOBAL_SCALE_2D, 
                3 / camera.zoom, 
                RED
            );
        }
    }
    EndMode2D();
}

Scene GearScene(float _z1 = 16.0f, float _m1 = 1.0f, float _angle = 20.0f){
    Scene scene;
    float 
    m = _m1,
    angle = _angle,
    z1 = _z1,
    z2 = 300.0f,
    dp = m * std::numbers::pi,
    dpd = 360.0f / z1,
    d1 = m * z1,
    d2 = m * z2,
    db1 = d1 * std::cos(VecUtils::degToRad(angle)),
    db2 = d2  * std::cos(VecUtils::degToRad(angle)),
    r1 = d1 / 2.0f,
    r2 = d2 / 2.0f,
    ra1 = r1 + m,
    ra2 = r2 + m,
    rb1 = (db1 / 2.0f),
    rb2 = db2 / 2.0f,
    rf1 = r1 - 1.25f * m,
    rf2 = r2 - 1.25 * m,
    s = dp / 2.0f,
    a = (d1 + d2) / 2.0f;

    Vec3 
    p1 = Vec3(-(a / 2)),
    p2 = Vec3(a / 2),
    p = Vec3(-(a / 2) + r1),
    collisionLineFirstPoint = VecUtils::rotatePointAtPoint(Vec3(0.0f, 10000.0f), Vec3(), angle),
    collisionLineSecondPoint = VecUtils::rotatePointAtPoint(Vec3(0.0f, -10000.0f), Vec3(), angle);
    collisionLineFirstPoint.x += p.x;
    collisionLineSecondPoint.x += p.x;

    Vec3 centerPoint = VecUtils::perpendicularToLine(p1, collisionLineFirstPoint, collisionLineSecondPoint);

    float distPoints = VecUtils::dist(p, centerPoint) / 100;

    std::vector<Vec3> involutePoints;

    // scene.addAnimationStep([debugPoint](Scene& scene){
    //     float max = 50;
    //     scene.get<FigurePoint>(debugPoint)->pos.x += 0.1;
    //     if(scene.get<FigurePoint>(debugPoint)->pos.x < max){
    //         return false;
    //     } else {
    //         return true;
    //     }
    // });
    
    for(float i = 100; i > -300; i -= 5){
        Vec3 refPoint = VecUtils::movePointAtLineByPercent(centerPoint, p, i);
        float angle = VecUtils::getArcAngleByArcLength(rb1, distPoints * i);
        Vec3 pointAtArc = VecUtils::rotatePointAtPoint(centerPoint, p1, -angle);
        Vec3 pointAtRadius = VecUtils::movePointAtLineByScalar(pointAtArc, p1, VecUtils::dist(refPoint, p));
        Vec3 pointAtInvolute = VecUtils::rotatePointAtPoint(pointAtRadius, pointAtArc, 90.0f);
        involutePoints.push_back(pointAtInvolute);
    }

    //точка пересечения окружности вершин и эвольвенты
    std::vector<Vec3> hasFindedInvoluteTopPoint = VecUtils::findIntersectionCircleAndPath(p1, ra1, involutePoints);
    
    Vec3 topPointInvolute;
    
    if(hasFindedInvoluteTopPoint.size()){
        topPointInvolute = hasFindedInvoluteTopPoint[0];
        involutePoints.push_back(topPointInvolute);
    } else {
        TraceLog(LOG_INFO, "Не найдена точка пересечения эвольвенты и окружности вершин зубьев");
    }
    
    std::vector<Vec3> hasFindedInvoluteBottomPoint = VecUtils::findIntersectionCircleAndPath(p1, rf1, involutePoints);
    
    Vec3 bottomPointInvolute;
    
    if(hasFindedInvoluteBottomPoint.size()){
        bottomPointInvolute = hasFindedInvoluteBottomPoint[0];
        involutePoints.insert(involutePoints.begin(), bottomPointInvolute);
    } else {
        TraceLog(LOG_INFO, "Не найдена точка пересечения эвольвенты и окружности впадин зубьев");
    }
    
    std::erase_if(involutePoints, [p1, ra1, rf1](Vec3 point){
        float dist = VecUtils::dist(p1, point);
        return (dist > ra1 + LOCAL_EPSILON) || (dist < rf1 - LOCAL_EPSILON);
    });

     std::vector<Vec3> hasFindedIntersectionInvoluteAndPitchCircle = VecUtils::findIntersectionCircleAndPath(p1, r1, involutePoints);
     
     Vec3 intersecInvoluteAndCircle;
     
    if(hasFindedIntersectionInvoluteAndPitchCircle.size()){
         intersecInvoluteAndCircle = hasFindedIntersectionInvoluteAndPitchCircle[0];
        } else {
            TraceLog(LOG_ERROR, "Не найдена точка пересечения эвольвенты и делительной окружности");
            return scene;
    }
        
    float symmetryAngle = VecUtils::getArcAngleByArcLength(r1, s) / 2;
    Vec3 symmetryPoint = VecUtils::rotatePointAtPoint(intersecInvoluteAndCircle, p1, symmetryAngle);
    Vec3 symmetryLineStart = VecUtils::movePointAtLineByPercent(symmetryPoint, p1, 20);
    Vec3 symmetryLineEnd = VecUtils::movePointAtLineByPercent(symmetryPoint, p1, -100);
    
    std::vector<Vec3> mirroredPoints;

    for(int i = involutePoints.size() - 1; i > -1; i--){
           Vec3 mirrored = VecUtils::symmetryMirror(involutePoints[i], symmetryLineStart, symmetryLineEnd);
           mirroredPoints.push_back(mirrored);
    }

    involutePoints.insert(involutePoints.end(), mirroredPoints.begin(), mirroredPoints.end());
       
    /**
    * нижняя точка профиля зуба - либо точка лежашая на делительной окружности, либо на окружности впадин (если была добавлена)
     */
    Vec3 bottomPoint;

    int minIdx;
    float minDist = 999999999.0f;
    for(int i = 0; i < involutePoints.size(); i++){
        float dist = VecUtils::dist(p1, involutePoints[i]);
        if(dist < minDist){
            minDist = dist;
            minIdx = i;
        }
    }

    bottomPoint = involutePoints[minIdx];

    //точка пересечения окружности впадин и эвольвенты
    std::vector<Vec3> hasFindedInvolutePointIntersectedWithRefCircle = VecUtils::findIntersectionLineAndCircle(p1, bottomPoint, p1, rf1);
    
    
    Vec3 involuteBottomPoint;
    
    if(hasFindedInvolutePointIntersectedWithRefCircle.size()){
        involuteBottomPoint = hasFindedInvolutePointIntersectedWithRefCircle[0];
    } else {
        TraceLog(LOG_ERROR, "Не найдена точка пересечения линии основания зуба и окружности впадин зубьев");
        return scene;
    }

    /** точки находящиеся во впадине шестерни */
    std::vector<Vec3> bottomCirclePoints;

    Vec3 referencePointToAngle = VecUtils::perpendicularToLine(bottomPoint, symmetryLineStart, symmetryLineEnd);



    if(involuteBottomPoint.y > referencePointToAngle.y){
        Vec3 symmetryPoint = VecUtils::symmetryMirror(involuteBottomPoint, symmetryLineStart, symmetryLineEnd);
        Vec3 rotatedPoint = VecUtils::rotatePointAtPoint(symmetryPoint, p1, dpd);
        float extremePointsAngle = VecUtils::angleByPoints(p1, involuteBottomPoint, rotatedPoint);
        for (float angle = 0; angle < extremePointsAngle; angle += 0.5f) {
            Vec3 point = VecUtils::rotatePointAtPoint(involuteBottomPoint, p1, angle);
            bottomCirclePoints.push_back(point);
        }
    } else {
        //Точка находящееся у корня зуба
        Vec3 point = VecUtils::symmetryMirror(involuteBottomPoint, symmetryLineStart, symmetryLineEnd);
        bottomCirclePoints.push_back(point);

        Vec3 rotatedPoint = VecUtils::rotatePointAtPoint(involuteBottomPoint, p1, dpd);
        float extremePointsAngle = VecUtils::angleByPoints(p1, rotatedPoint, point);
        for (float angle = extremePointsAngle; angle > 0; angle -= 0.5f) {
            float refAngle = VecUtils::getArcAngleByArcLength(r1, dp);
            Vec3 point = VecUtils::rotatePointAtPoint(involuteBottomPoint, p1, -angle + refAngle);
            bottomCirclePoints.push_back(point);
        }

    }



    if(involuteBottomPoint.y > referencePointToAngle.y){
        //Точка находящееся у корня зуба
        Vec3 point = VecUtils::symmetryMirror(involuteBottomPoint, symmetryLineStart, symmetryLineEnd);
        involutePoints.insert(involutePoints.begin(), point);
    } else {
        //Точка находящееся у корня зуба
        involutePoints.insert(involutePoints.begin(), involuteBottomPoint);
    }

    involutePoints.insert(involutePoints.end(), bottomCirclePoints.begin(), bottomCirclePoints.end());

    // for(int i = 0; i < involutePoints.size(); i++){
    //    scene.push(FigurePoint(involutePoints[i]));
    // }

    std::vector<Vec3> result{};
    result.insert(result.begin(), involutePoints.begin(), involutePoints.end());

    for(int zi = 1; zi < z1; zi++){
        for(int idx = 0; idx < involutePoints.size(); idx++){
            Vec3 rotated = VecUtils::rotatePointAtPoint(involutePoints[idx], p1, dpd * zi);
            result.push_back(rotated);
        }
    }

    //Ставим ещё одну точку "в начало профиля", но в конце массива точек, чтобы профиль был замкнут
    if(involuteBottomPoint.y > referencePointToAngle.y){
        //Точка находящееся у корня зуба
        Vec3 point = VecUtils::symmetryMirror(involuteBottomPoint, symmetryLineStart, symmetryLineEnd);
         result.push_back(point);
    } else {
        //Точка находящееся у корня зуба
        Vec3 point = involuteBottomPoint;
        result.push_back(point);
    }


    for(int i = 0; i < result.size(); i++){
        Vec3 point = result[i];
        point -= p1;
      scene.push(FigurePoint(point));
    }


    for(int i = 1; i < result.size(); i++){
        scene.push(FigureLine(result[i -1], result[i]));
    }


    return scene;

}

Scene RackScene(float z = 1.0f, float m = 1.0f, float angle = 20.0f){
    Scene scene;
    float 
    h1 = m,
    h2 = 1.25f * m,
    t = m * std::numbers::pi,
    lt = t / 2;

    Vec3 triangleTopPoint = Vec3(0,0);
    Vec3 triangleBottomPoint = Vec3(0, -999);
    Vec3 triangleLeftPoint = VecUtils::rotatePointAtPoint(triangleBottomPoint, triangleTopPoint, -angle);
    Vec3 triangleRightPoint = VecUtils::rotatePointAtPoint(triangleBottomPoint, triangleTopPoint, angle);

    // Vec3 centerPoint = Vec3(0, h2);
    // Vec3 refTopPoint = Vec3(0, h2 + h1);

    // float offset = VecUtils::rotatePointAtPoint(centerPoint, refTopPoint, angle).x;
    // Vec3 halfTopLeftPoint = Vec3(-offset, h2);
    // Vec3 movedHalfTopLeft = VecUtils::movePointAtLineByScalar(refTopPoint, halfTopLeftPoint, 999);
    // Vec3 halfTopRightPoint = Vec3(offset, h2);
    // Vec3 movedHalfTopRight = VecUtils::movePointAtLineByScalar(refTopPoint, halfTopRightPoint, 999);
    
    // scene.push(FigurePoint(centerPoint));
    // scene.push(FigurePoint(refTopPoint));
    // scene.push(FigurePoint(halfTopLeftPoint));
    // scene.push(FigurePoint(halfTopRightPoint));

    // std::optional<Vec3> bottomLeftPointRef = VecUtils::findIntersectionLineAndLine(refTopPoint, movedHalfTopLeft, Vec3(-999, 0), Vec3(999, 0));
    // std::optional<Vec3> bottomRightPointRef = VecUtils::findIntersectionLineAndLine(refTopPoint, movedHalfTopRight, Vec3(-999, 0), Vec3(999, 0));

    // if(bottomLeftPointRef && bottomRightPointRef){
    //     Vec3 bottomLeftPoint = bottomLeftPointRef.value();
    //     Vec3 bottomRightPoint = bottomRightPointRef.value();
    //     scene.push(FigurePoint(bottomLeftPoint));
    //     scene.push(FigurePoint(bottomRightPoint));
    // }

    // Vec3 topRefLeft = VecUtils::movePointAtLineByScalar(halfTopLeftPoint, refTopPoint, 999);
    // Vec3 topRefRight = VecUtils::movePointAtLineByScalar(halfTopRightPoint, refTopPoint, 999);

    // std::optional<Vec3> topLeftPointRef = VecUtils::findIntersectionLineAndLine(halfTopLeftPoint, topRefLeft, Vec3(-999, h1 + h2), Vec3(999, h1 + h2));
    // std::optional<Vec3> topRightPointRef = VecUtils::findIntersectionLineAndLine(halfTopRightPoint, topRefRight, Vec3(-999, h1 + h2), Vec3(999, h1 + h2));

    // if(topLeftPointRef && topRightPointRef){
    //     Vec3 topLeftPoint = topLeftPointRef.value();
    //     Vec3 topRightPoint = topRightPointRef.value();
    //     scene.push(FigurePoint(topLeftPoint));
    //     scene.push(FigurePoint(topRightPoint));
    // }
    


    







    


    return scene;

}


Mesh buildRadialTopology(std::vector<Vec3>& points, float modelExtrudeSize = 1.0f){
    Mesh mesh = { 0 };
    int POINTS_SIZE = points.size();
    std::vector<Vec3> pointsRotated = points;

    // WIP: для косозубых
    // for(int i = 0; i < pointsRotated.size(); i++){
    //    pointsRotated[i] = VecUtils::rotatePointAtPoint(pointsRotated[i], Vec3(), 10);
    //    pointsRotated[i].z = modelExtrudeSize;
    // }


    float extrudeSize = modelExtrudeSize;
    mesh.vertexCount = POINTS_SIZE + POINTS_SIZE + POINTS_SIZE + POINTS_SIZE + POINTS_SIZE + POINTS_SIZE + 1 + 1; 
    mesh.triangleCount = POINTS_SIZE + POINTS_SIZE + POINTS_SIZE + POINTS_SIZE; 

    mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.normals = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    int centerBottomIdx = mesh.vertexCount - 2;
    int centerTopIdx = mesh.vertexCount - 1;
    int ringStartIdx = 1;

    int offset = 0;
    int topVerticesOffset = 0;
    int topVerticesAnotherNormalOffset = 0;
    int bottomVerticesOffset = 0;
    int bottomVerticesAnotherNormalOffset = 0;

    //вершины верхней части с нормалями направленными вверх
    for (int i = 0; i < POINTS_SIZE; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 point = pointsRotated[i];

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = extrudeSize; 
        mesh.vertices[z] = point.x;

        mesh.normals[x] = 0.0f;
        mesh.normals[y] = 1.0f;
        mesh.normals[z] = 0.0f;
    }

    
    offset += POINTS_SIZE;
    topVerticesOffset = offset;
    //вершины верхней части с нормалями направленными вбок в одну сторону
    for (int i = 0; i < POINTS_SIZE - 1; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 point = pointsRotated[i];
        Vec3 bottomPoint = points[i];
        Vec3 nextPoint =  pointsRotated[i + 1];
        Vec3 a = {point.x, point.y, point.z};
        Vec3 c = {nextPoint.x, nextPoint.y, -extrudeSize};
        Vec3 b = {nextPoint.x, nextPoint.y, extrudeSize};
        a -= c;
        b -= c;

        float xnormal = (a.y * b.z) - (a.z * b.y);
        float ynormal = (a.z * b.x) - (a.x * b.z);
        float znormal = (a.x * b.y) - (a.y * b.x);

        Vec3 normal = Vec3(xnormal, ynormal, znormal);
        normal.normalize();

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = extrudeSize;
        mesh.vertices[z] = point.x;

        mesh.normals[x] = -normal.y;
        mesh.normals[y] = znormal;
        mesh.normals[z] = -normal.x;
    }

    offset += POINTS_SIZE;
    topVerticesAnotherNormalOffset = offset;
    //вершины верхней части с нормалями направленными вбок в другую сторону
    for (int i = 0; i < POINTS_SIZE; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 prevPoint = pointsRotated[i - 1];
        Vec3 point = pointsRotated[i];
        Vec3 a = {point.x, point.y, extrudeSize};
        Vec3 c = {point.x, point.y, -extrudeSize};
        Vec3 b = {prevPoint.x, prevPoint.y, extrudeSize};
        a -= c;
        b -= c;

        float xnormal = (a.y * b.z) - (a.z * b.y);
        float ynormal = (a.z * b.x) - (a.x * b.z);
        float znormal = (a.x * b.y) - (a.y * b.x);

        Vec3 normal(xnormal, ynormal, znormal);
        normal.normalize();

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = extrudeSize;
        mesh.vertices[z] = point.x;

        mesh.normals[x] = normal.y;
        mesh.normals[y] = znormal;
        mesh.normals[z] = normal.x;
    }

    offset += POINTS_SIZE;
    bottomVerticesOffset = offset;

    //вершины нижней части с нормалями направленными вбок в одну сторону
    for (int i = 0; i < POINTS_SIZE; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 point = points[i];
        Vec3 nextPoint =  points[i + 1];
        Vec3 a = {point.x, point.y, -extrudeSize};
        Vec3 c = {nextPoint.x, nextPoint.y, -extrudeSize * 2};
        Vec3 b = {nextPoint.x, nextPoint.y, -extrudeSize};
        a -= c;
        b -= c;

        float xnormal = (a.y * b.z) - (a.z * b.y);
        float ynormal = (a.z * b.x) - (a.x * b.z);
        float znormal = (a.x * b.y) - (a.y * b.x);

        Vec3 normal(xnormal, ynormal, znormal);
        normal.normalize();

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = -extrudeSize;
        mesh.vertices[z] = point.x;

        mesh.normals[x] = -normal.y;
        mesh.normals[y] = znormal;
        mesh.normals[z] = -normal.x;
    }

    offset += POINTS_SIZE;
    bottomVerticesAnotherNormalOffset = offset;
    //вершины нижней части с нормалями направленными вбок в другую сторону
    for (int i = 0; i < POINTS_SIZE; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 prevPoint = points[i - 1];
        Vec3 point = points[i];
        Vec3 a = {point.x, point.y, -extrudeSize};
        Vec3 c = {point.x, point.y, -extrudeSize * 2};
        Vec3 b = {prevPoint.x, prevPoint.y, -extrudeSize};
        a -= c;
        b -= c;

        float xnormal = (a.y * b.z) - (a.z * b.y);
        float ynormal = (a.z * b.x) - (a.x * b.z);
        float znormal = (a.x * b.y) - (a.y * b.x);

        Vec3 normal(xnormal, ynormal, znormal);
        normal.normalize();

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = -extrudeSize; 
        mesh.vertices[z] = point.x;

        mesh.normals[x] = normal.y;
        mesh.normals[y] = znormal;
        mesh.normals[z] = normal.x;
    }


    offset += POINTS_SIZE;
    int bottomVerticesLookAtbottomOffset = offset;
    //вершины нижней части с нормалями направленными вниз
    for (int i = 0; i < POINTS_SIZE; i++) {
        int vIdx = offset + i;
        int x = vIdx * 3 + 0;
        int y = vIdx * 3 + 1;
        int z = vIdx * 3 + 2;

        Vec3 point = points[i];

        mesh.vertices[x] = point.y;
        mesh.vertices[y] = -extrudeSize; 
        mesh.vertices[z] = point.x;

        mesh.normals[x] = 0.0f;
        mesh.normals[y] = -1.0f;
        mesh.normals[z] = 0.0f;
    }

    

    mesh.vertices[centerTopIdx * 3 + 0] = 0.0f;
    mesh.vertices[centerTopIdx * 3 + 1] = extrudeSize;
    mesh.vertices[centerTopIdx * 3 + 2] = 0.0f;

    mesh.normals[centerTopIdx * 3 + 0] = 0.0f;
    mesh.normals[centerTopIdx * 3 + 1] = 1.0f;
    mesh.normals[centerTopIdx * 3 + 2] = 0.0f;

    mesh.vertices[centerBottomIdx * 3 + 0] = 0.0f;
    mesh.vertices[centerBottomIdx * 3 + 1] = -extrudeSize;
    mesh.vertices[centerBottomIdx * 3 + 2] = 0.0f;

    mesh.normals[centerBottomIdx * 3 + 0] = 0.0f;
    mesh.normals[centerBottomIdx * 3 + 1] = -1.0f;
    mesh.normals[centerBottomIdx * 3 + 2] = 0.0f;
    
    //верхняя плоскость
    int indexCounter = 0;

    for (int i = 0; i < POINTS_SIZE; i++) {
        int currentRingVert = ringStartIdx + i;
        int nextRingVert = ringStartIdx + (
            (i + 1) != POINTS_SIZE 
            ? i + 1
            : 0); 
        
        
        mesh.indices[indexCounter++] = centerTopIdx;
        mesh.indices[indexCounter++] = currentRingVert;  
        mesh.indices[indexCounter++] = nextRingVert;
    }

    // первая очередь треугольников
    for (int i = 0; i < POINTS_SIZE - 1; i++)  {
        int currentRingVert = topVerticesOffset + i;
        int nextRingVert = (i + 1) != POINTS_SIZE ? topVerticesAnotherNormalOffset + i + 1 : topVerticesAnotherNormalOffset + 1;
        int bottomVert = bottomVerticesOffset + i;

        mesh.indices[indexCounter++] = bottomVert;  
        mesh.indices[indexCounter++] = nextRingVert;
        mesh.indices[indexCounter++] = currentRingVert;
    }

    // вторая очередь треугольников
    for (int i = 0; i < POINTS_SIZE; i++)  {
        int currentRingVert = bottomVerticesOffset + i;
        int nextRingVert = (i + 1) != POINTS_SIZE ? bottomVerticesAnotherNormalOffset + i + 1: bottomVerticesAnotherNormalOffset + 0;
        int topVert = (i + 1) != POINTS_SIZE ? topVerticesAnotherNormalOffset + i + 1 : topVerticesAnotherNormalOffset;
        mesh.indices[indexCounter++] = currentRingVert;
        mesh.indices[indexCounter++] = nextRingVert;
        mesh.indices[indexCounter++] = topVert;  
    }

    //нижняя плоскость
    for (int i = 0; i < POINTS_SIZE - 1; i++) {
        int currentRingVert = bottomVerticesLookAtbottomOffset + i;
        int nextRingVert = (i + 1) != POINTS_SIZE ? bottomVerticesLookAtbottomOffset + i + 1 : bottomVerticesLookAtbottomOffset + 1;
        
        mesh.indices[indexCounter++] = nextRingVert;
        mesh.indices[indexCounter++] = currentRingVert;  
        mesh.indices[indexCounter++] = centerBottomIdx;
    }

    return mesh;

    }


void drawMeshNormals(const Mesh& mesh){
    for(int i = 0; i < mesh.vertexCount; i++){
        Vector3 startPos = { mesh.vertices[i*3], mesh.vertices[i*3+1], mesh.vertices[i*3+2] };
        Vector3 normalDir = { mesh.normals[i*3], mesh.normals[i*3+1], mesh.normals[i*3+2] };

        // Увеличиваем нормаль для видимости
        float scale = 0.2f;
        Vector3 endPos = { 
            startPos.x + normalDir.x * scale, 
            startPos.y + normalDir.y * scale, 
            startPos.z + normalDir.z * scale 
        };

        DrawLine3D(startPos, endPos, GREEN);
    }
}


void drawVerticeByIndex(const Mesh& mesh, int idx, float scale = 1.0f){
    Vector3 verticeDir = { 
        mesh.vertices[idx*3], 
        mesh.vertices[idx*3+1], 
        mesh.vertices[idx*3+2] 
    };

    DrawCube(verticeDir, 0.001f * scale, 0.001f * scale, 0.001f * scale, RED);

    Vector3 normalDir = { 
        mesh.normals[idx*3], 
        mesh.normals[idx*3+1], 
        mesh.normals[idx*3+2] 
    };
    Vector3 startPos = { 
        mesh.vertices[idx*3], 
        mesh.vertices[idx*3+1], 
        mesh.vertices[idx*3+2] 
    };
    Vector3 endPos = { 
        startPos.x + normalDir.x * scale * 0.01f, 
        startPos.y + normalDir.y * scale * 0.01f, 
        startPos.z + normalDir.z * scale * 0.01f
    };

    DrawLine3D(startPos, endPos, GREEN);    
}

std::tuple<Mesh, Model> buildModel(Scene& scene, Shader& shader, float extrudeSize = 1.0f){
    std::vector<Vec3> scenePoints = scene.getPoints();
    Mesh mesh = buildRadialTopology(scenePoints, extrudeSize);
    UploadMesh(&mesh, false);
    Model model = LoadModelFromMesh(mesh);
    model.materials[0].shader = shader;
    return std::tuple(mesh, model);
}

#if defined(PLATFORM_WEB)
void saveWebSTLFile(Mesh &gearMesh){
    std::string stl;

    stl += "solid gear\n";   
    for(int i = 0; i < gearMesh.triangleCount; i++){
        std::string xnormal = TextFormat("%.6f",gearMesh.normals[i * 3]);
        std::string ynormal = TextFormat("%.6f",gearMesh.normals[(i * 3) + 1]);
        std::string znormal = TextFormat("%.6f",gearMesh.normals[(i * 3) + 2]);
        int v1idx = gearMesh.indices[i * 3];
        int v2idx = gearMesh.indices[(i * 3) + 1];
        int v3idx = gearMesh.indices[(i * 3) + 2]; 
        std::string v1x = TextFormat("%.6f",gearMesh.vertices[(v1idx * 3)]);
        std::string v1y = TextFormat("%.6f",gearMesh.vertices[(v1idx * 3) + 1]);
        std::string v1z = TextFormat("%.6f",gearMesh.vertices[(v1idx * 3) + 2]);
        std::string v2x = TextFormat("%.6f",gearMesh.vertices[(v2idx * 3)]);
        std::string v2y = TextFormat("%.6f",gearMesh.vertices[(v2idx * 3) + 1]);
        std::string v2z = TextFormat("%.6f",gearMesh.vertices[(v2idx * 3) + 2]);
        std::string v3x = TextFormat("%.6f",gearMesh.vertices[(v3idx * 3)]);
        std::string v3y = TextFormat("%.6f",gearMesh.vertices[(v3idx * 3) + 1]);
        std::string v3z = TextFormat("%.6f",gearMesh.vertices[(v3idx * 3) + 2]);
        
        stl += "facet normal ";
        stl += xnormal;
        stl += " ";
        stl += ynormal;
        stl += " ";
        stl += znormal; 
        stl += "\n"; 
        stl += "outer loop\n";
        stl += "vertex ";
        stl += v1x;
        stl += " ";
        stl += v1y;
        stl += " ";
        stl += v1z;
        stl += "\n";
        stl += "vertex ";
        stl += v2x;
        stl += " ";
        stl += v2y;
        stl += " ";
        stl += v2z;
        stl += "\n";
        stl += "vertex ";
        stl += v3x;
        stl += " ";
        stl += v3y;
        stl += " ";
        stl += v3z;
        stl += "\n";
        stl += "endloop\n";
        stl += "endfacet\n";
    }
    stl += "endsolid gear\n";
    emscripten::val::global().call<void>("downloadAsSTL", stl);
}
#else
void saveSTLFile(Mesh &gearMesh, std::string name){
    std::ofstream outputFile( name + ".stl");
    outputFile << "solid gear\n" << std::fixed << std::setprecision(6);   
    for(int i = 0; i < gearMesh.triangleCount; i++){
        float xnormal = gearMesh.normals[i * 3];
        float ynormal = gearMesh.normals[(i * 3) + 1];
        float znormal = gearMesh.normals[(i * 3) + 2];
        int v1idx = gearMesh.indices[i * 3];
        int v2idx = gearMesh.indices[(i * 3) + 1];
        int v3idx = gearMesh.indices[(i * 3) + 2]; 
        float v1x = gearMesh.vertices[(v1idx * 3)];
        float v1y = gearMesh.vertices[(v1idx * 3) + 1];
        float v1z = gearMesh.vertices[(v1idx * 3) + 2];
        float v2x = gearMesh.vertices[(v2idx * 3)];
        float v2y = gearMesh.vertices[(v2idx * 3) + 1];
        float v2z = gearMesh.vertices[(v2idx * 3) + 2];
        float v3x = gearMesh.vertices[(v3idx * 3)];
        float v3y = gearMesh.vertices[(v3idx * 3) + 1];
        float v3z = gearMesh.vertices[(v3idx * 3) + 2];

        outputFile << "facet normal " << xnormal << " " << ynormal << " " << znormal << "\n"; 
        outputFile << "outer loop\n";
        outputFile << "vertex " << v1x << " " << v1y << " " << v1z << "\n";
        outputFile << "vertex " << v2x << " " << v2y << " " << v2z << "\n";
        outputFile << "vertex " << v3x << " " << v3y << " " << v3z << "\n";
        outputFile << "endloop\n";
        outputFile << "endfacet\n";
    }
    outputFile << "endsolid gear\n";

    outputFile.close();
}
#endif

Font loadFonts(){
    // 1. Создаем массив нужных кодовых точек
    int codepoints[512] = { 0 };
    int index = 0;

    // Базовый ASCII (латиница, цифры, символы) от 32 до 126
    for (int i = 0; i < 95; i++) {
        codepoints[index++] = 32 + i;
    }

    // Кириллица (А-Я, а-я, Ё, ё) в диапазоне Unicode 0x0400 - 0x04FF
    for (int i = 0; i < 255; i++) {
        codepoints[index++] = 0x400 + i; 
    }

    return LoadFontEx("resources/fonts/MegaSans.ttf", 48, codepoints, index);
}



int main(void)
{

    /** отладка профиля */
    bool mode = true;

    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "geardemo");
    Font font = loadFonts();
    Camera2D camera = { 0 };
    camera.zoom = 1.0f;

    Camera3D orbitCam;
    orbitCam.fovy = 45;
    orbitCam.target = Vector3{0, 0 ,0 };
    orbitCam.position = Vector3{ 0, 0 ,5 };
    orbitCam.up = Vector3{ 0,1,0 };
    float camDist = 20;
    float rotAngle = 0;
    float tiltAngle = -45; 
    float rotSpeed = 0.5f; 

    
    SetTargetFPS(60);   
    float time = 0.0f;     
           

    Shader shader = LoadShader(TextFormat("resources/shaders/lighting.vs"),
                               TextFormat("resources/shaders/fog.fs"));

    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    Vector4 ambient = (Vector4){ 0.1f, 0.1f, 0.1f, 1.0f };
    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, &ambient, SHADER_UNIFORM_VEC4);

    Vector3 lightPos = Vector3{ 1, 0, 5};
    int lightPosLoc = GetShaderLocation(shader, "lightPos");
    SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);

    // Vector4 fogColor = ColorNormalize(GRAY);
    // int fogColorLoc = GetShaderLocation(shader, "fogColor");
    // SetShaderValue(shader, fogColorLoc, &fogColor, SHADER_UNIFORM_VEC4);

    float fogDensity = 0.001f;
    int fogDensityLoc = GetShaderLocation(shader, "fogDensity");
    SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
    Light light = CreateLight(LIGHT_DIRECTIONAL, (Vector3){ 0, 10, 6 }, Vector3Zero(), WHITE, shader);



    int count = 17;
    int nextCount = 17;
    int angle = 20;
    int nextAngle = 20;
    int gearModule = 1;
    int nextGearModule = 1;
    int extrudeSize = 3;
    int nextExtrudeSize = 3;

    Scene scene = GearScene(count, gearModule, angle);
    Scene rack = RackScene();
    auto [gearMesh, gearModel] = buildModel(scene, shader, extrudeSize);

    // rlDisableBackfaceCulling();

    while (!WindowShouldClose())
    {
        time += GetFrameTime();

        if(
            count != nextCount ||
            angle != nextAngle ||
            gearModule != nextGearModule ||
            extrudeSize != nextExtrudeSize
        ){
            count = nextCount;
            angle = nextAngle;
            gearModule = nextGearModule;
            extrudeSize = nextExtrudeSize;

            scene = GearScene(count, gearModule, angle);
            UnloadModel(gearModel);
            auto [_gearMesh, _gearModel] = buildModel(scene, shader, extrudeSize);
            gearMesh = _gearMesh;
            gearModel = _gearModel;
        }




        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
            Vector2 newPos = GetMouseDelta();
            if(IsKeyDown(KEY_LEFT_CONTROL)){
                rotAngle -= (newPos.x * rotSpeed) / 15;
                tiltAngle -= (newPos.y * rotSpeed) / 15;
            } else {
                rotAngle -= newPos.x * rotSpeed;
                tiltAngle -= newPos.y * rotSpeed;
            }
            if(tiltAngle > 89){
            tiltAngle = 89;
            };
            if(tiltAngle < -89){
            tiltAngle = -89;
            };
        }

        if(IsKeyDown(KEY_LEFT_CONTROL)){
            camDist -= GetMouseWheelMove() / 15;

        } else {
            camDist -= GetMouseWheelMove();
        }

        if (camDist < 1){
            camDist = 1;
        }

        Vector3 camPos = {0, 0, camDist};
        Vector3 lightPos= {-1, 1, camDist};
        Matrix tiltMat = MatrixRotateX(VecUtils::degToRad(tiltAngle));
        Matrix rotMat = MatrixRotateY(VecUtils::degToRad(rotAngle));
        Matrix mat = MatrixMultiply(tiltMat, rotMat);

        camPos = Vector3Transform(camPos, mat);
        lightPos = Vector3Transform(lightPos, mat);
        orbitCam.target = Vector3();
        orbitCam.position = camPos;

        SetShaderValue(shader, fogDensityLoc, &fogDensity, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shader, lightPosLoc, &lightPos, SHADER_UNIFORM_VEC3);
        SetShaderValue(shader, shader.locs[SHADER_LOC_VECTOR_VIEW], &orbitCam.position.x, SHADER_UNIFORM_VEC3);

        
        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        if(!mode){
            updateCameraByMouseControl(camera);
            drawGridByCurrentCamera(camera);
            renderScene(camera, rack);
            drawMouseCoords(camera);
        } else {
            BeginMode3D(orbitCam);
            DrawModel(gearModel, Vector3Zero(), 1.0f, GRAY);
            EndMode3D();

            DrawTextEx(font, "количество зубьев", (Vector2){ 10, 10 }, 24, 2, GRAY);
            GuiSpinner((Rectangle){ 10, 36, 120, 30 }, "",  &nextCount, 6, 1000, false);
            DrawTextEx(font, "угол зацепления", (Vector2){ 10, 70 }, 24, 2, GRAY);
            GuiSpinner((Rectangle){ 10, 96, 120, 30 }, "",  &nextAngle, 7, 30, false);
            DrawTextEx(font, "модуль зуба", (Vector2){ 10, 130 }, 24, 2, GRAY);
            GuiSpinner((Rectangle){ 10, 156, 120, 30 }, "",  &nextGearModule, 1, 100, false);
            DrawTextEx(font, "высота шестерни", (Vector2){ 10, 190 }, 24, 2, GRAY);
            GuiSpinner((Rectangle){ 10, 216, 120, 30 }, "",  &nextExtrudeSize, 1, 100, false);
            if (GuiButton((Rectangle){ 10, 256, 120, 30 }, "")) {
                #if defined(PLATFORM_WEB)
                    saveWebSTLFile(gearMesh);
                #else 
                   std::string meshName = "gear_";
                   meshName += "m";
                   meshName += TextFormat("%d", nextGearModule);
                   meshName += "_";
                   meshName += "a";
                   meshName += TextFormat("%d", nextAngle);
                   meshName += "_";
                   meshName += "z";
                   meshName += TextFormat("%d", nextCount);

                    saveSTLFile(gearMesh, meshName);
                #endif

                
            }
            DrawTextEx(font, "Скачать STL", (Vector2){ 15, 264 }, 16, 2, GRAY);
        } 
        EndDrawing();
    }
    CloseWindow();        

    return 0;
}
