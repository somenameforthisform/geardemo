#pragma once
#include <numbers>
#include <cmath>
#include <optional>
#include <vector> 
#include "vecutils.h"
#include "vec3.h"

namespace VecUtils {

    /**
    * Возвращает растояние между двумя векторами
    */
    float dist(const Vec3& a, const Vec3& b);
    
    /**
    * Радианы в градусы
    */
    float radToDeg(float rad);

    /**
    * Градусы в радианы
    */
    float degToRad(float deg);

    /**
    * Перемщение вдоль линии от точки start до точки end на n c созданием новой точки
    */
    Vec3 movePointAtLineByScalar(const Vec3& start, const Vec3& end, float n);

    /**
    * Перемщение вдоль линии от точки start до точки end на n процентов от длины отрезка start-end c созданием новой точки 
    */
    Vec3 movePointAtLineByPercent(const Vec3& start, const Vec3& end, float n);

    /**
    * Возвращает угол дуги в градусах по радиусу r и длине дуги l 
    */
    float getArcAngleByArcLength(float r, float l);

    /**
    * Возвращает длину дуги по радиусу r и углу в градусах deg 
    */
    float getArcLength(float r, float deg);

    /**
    * Поворот вокруг центра координат
    */
    Vec3 rotate(const Vec3& point, float deg);

    /**
    * Вращает точку вокруг точки на deg градусов
    */
    Vec3 rotatePointAtPoint(const Vec3& point, const Vec3& target, float deg);

    /**
    * Вращает точку вокруг окружности на дистанцию n
    */
    Vec3 moveAtCircle(const Vec3& point, const Vec3& circleCenter, float circleRadius, float n);

    /**
    * Находит пересечение линии и линии аналитическим способом (через решение системы уравнений)
    */
    std::optional<Vec3> findIntersectionLineAndLine(
        const Vec3& startFirst, 
        const Vec3& endFirst, 
        const Vec3& startSecond, 
        const Vec3& endSecond
    );

    /**
    * Находит пересечение окружности и линии аналитическим способом (через решение системы уравнений)
    */
    std::vector<Vec3> findIntersectionLineAndCircle(
        const Vec3& lineFirstPoint,
        const Vec3& lineSecondPoint,
        const Vec3& circlePoint,
        float circleRadius,
        bool flag = false
    );

    /**
    * Находит пересечние линии и ломанной состоящей из множества точек
    */
    std::vector<Vec3> findIntersectionLineAndPath(
        const Vec3& lineFirstPoint, 
        const Vec3& lineSecondPoint, 
        const std::vector<Vec3>& path
    );

    /** 
    * Находит пересечение окружности и ломанной состоящей из множества точек
    */
    std::vector<Vec3> findIntersectionCircleAndPath(
        const Vec3& centerPoint, 
        float radius, 
        const std::vector<Vec3>& path
    );

    /**
    * Получить угол по трём точкам; center - точка угла
    */
    float angleByPoints(const Vec3& center, const Vec3& a, const Vec3& b);

    /**
    * Построить перпендикуляр из точки point в отрезок a-b
    */
    Vec3 perpendicularToLine(const Vec3& point, const Vec3& a, const Vec3& b);

    /**
    * Отражает точку симметрично относительно указанного отрезка lineFirstPoint-lineSecondPoint
    */
    Vec3 symmetryMirror(const Vec3& point, const Vec3& lineFirstPoint, const Vec3& lineSecondPoint);
}