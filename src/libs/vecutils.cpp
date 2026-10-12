#include <numbers>
#include <cmath>
#include <optional> 
#include "vecutils.h"
#include "vec3.h"

namespace VecUtils {
    const float epsilon =  0.01f;

    /**
    * Возвращает растояние между двумя векторами
    */
    float dist(const Vec3& a, const Vec3& b){
        return std::sqrt(std::pow(a.x -b.x, 2.0f) + std::pow(a.y - b.y, 2.0f));
    };

    /**
    * Радианы в градусы
    */
    float radToDeg(float rad){
        return rad / (std::numbers::pi / 180.0f);
    }

    /**
    * Градусы в радианы
    */
    float degToRad(float deg){
        return (deg * std::numbers::pi) / 180.0f;
    }

    /**
    * Перемщение вдоль линии от точки start до точки end на n c созданием новой точки
    */
    Vec3 movePointAtLineByScalar(const Vec3& start, const Vec3& end, float n){
        Vec3 p = Vec3(end);
        p -= start;
        float size = dist(Vec3(), p);
        p /= size;
        p *= n;
        Vec3 result = Vec3(start) += p;
        return result;
    }

    /**
    * Перемщение вдоль линии от точки start до точки end на n процентов от длины отрезка start-end c созданием новой точки 
    */
    Vec3 movePointAtLineByPercent(const Vec3& start, const Vec3& end, float n){
        Vec3 p = Vec3(end);
        p -= start;
        float size = dist(Vec3(), p);
        float len = size / 100.0f;
        p /= size;
        p *= (len * n);
        Vec3 result = Vec3(start) += p;
        return result;
    }

    /**
    * Возвращает угол дуги в градусах по радиусу r и длине дуги l 
    */
    float getArcAngleByArcLength(float r, float l) {
        return (l * 180.0f) / (std::numbers::pi * r);
    }

    /**
    * Возвращает длину дуги по радиусу r и углу в градусах deg 
    */
    float getArcLength(float r, float deg){
        return std::numbers::pi * deg / 180.0f;
    }

    /**
    * Поворот вокруг центра координат
    */
    Vec3 rotate(const Vec3& point, float deg){
        float dx = point.x * std::cos(degToRad(deg)) - point.y * std::sin(degToRad(deg));
        float dy = point.x * std::sin(degToRad(deg)) + point.y * std::cos(degToRad(deg));
        Vec3 result(dx, dy);
        return result;
    }

    /**
    * Вращает точку вокруг точки на deg градусов
    */
    Vec3 rotatePointAtPoint(const Vec3& point, const Vec3& target, float deg){
        float dx = (point.x - target.x) * std::cos(degToRad(deg)) - (point.y - target.y) * std::sin(degToRad(deg));
        float dy = (point.x - target.x) * std::sin(degToRad(deg)) + (point.y - target.y) * std::cos(degToRad(deg));
        Vec3 result(dx + target.x, dy + target.y);
        return result;
    }

    /**
    * Вращает точку вокруг окружности на дистанцию n
    */
    Vec3 moveAtCircle(const Vec3& point, const Vec3& circleCenter, float circleRadius, float n){
        float angle = getArcAngleByArcLength(circleRadius, n);
        Vec3 result = rotatePointAtPoint(point, circleCenter, angle);
        return result;
    }

    /**
    * Находит пересечение линии и линии аналитическим способом (через решение системы уравнений)
    */
    std::optional<Vec3> findIntersectionLineAndLine(
        const Vec3& startFirst, 
        const Vec3& endFirst, 
        const Vec3& startSecond, 
        const Vec3& endSecond){
            bool isFirstLineVertical = startFirst.x == endFirst.x;
            bool isSecondLineVertical = startSecond.x == endSecond.x;
            

            /**
            * Если обе линии вертикальны, то они либо лежат друг на друге хотя бы частично (а значит у всех точек будет одно значение "x")
            * либо у них нет точек пересечения
            */
            if(isFirstLineVertical && isSecondLineVertical && startFirst.x == startSecond.x){

                /*точка startFirst лежит на второй линии*/
                if(startFirst.y >= startSecond.y && startFirst.y <= endSecond.y ){
                    return Vec3(startFirst);
                }

                /*точка endFirst лежит на второй линии*/
                if(endFirst.y >= startSecond.y && endFirst.y <= endSecond.y){
                    return Vec3(startFirst);
                } 
                
                /* иначе они не пересекаются */
                return std::nullopt;
            }

            /** Если обе вертикальны и нет общего "x", значит нет точек пересечения */
            if(isFirstLineVertical && isSecondLineVertical){
                return std::nullopt;
            }

            /**
            * Если только одна линия вертикальна, то мы можем подставить "x" одной из точек вертикальной линии в уравнение прямой:
            * y = kx + b
            */
            if(isFirstLineVertical){
                float sdy = endSecond.y - startSecond.y;
                float sdx = endSecond.x - startSecond.x;
                float k = sdy/sdx;
                float b = startSecond.y - k * startSecond.x;
                float y = k * startFirst.x + b;

                /**
                * т.к. уравнение для бесконечной линии, то две непараллельные линии рано или поздно перескутся
                * здесь мы ограничиваем область определения найдя наименьший и наибольший "y"
                * далее будут аналогичные проверки
                */
                float minAY = std::min(startFirst.y, endFirst.y);
                float minBY = std::min(startSecond.y, endSecond.y);
                float maxAY = std::max(startFirst.y, endFirst.y);
                float maxBY = std::max(startSecond.y, endSecond.y);

                if(y + epsilon <= minAY || y + epsilon <= minBY){
                    return std::nullopt;
                }

                if(y - epsilon >= maxAY || y - epsilon >= maxBY){
                    return std::nullopt;
                }

                return Vec3(startFirst.x, y);
            }

            if(isSecondLineVertical){
                float fdy = endFirst.y - startFirst.y;
                float fdx = endFirst.x - startFirst.x;
                float k = fdy/fdx;
                float b = startFirst.y - k * startFirst.x;
                float y = k * startSecond.x + b;

                float minAY = std::min(startFirst.y, endFirst.y);
                float minBY = std::min(startSecond.y, endSecond.y);
                float maxAY = std::max(startFirst.y, endFirst.y);
                float maxBY = std::max(startSecond.y, endSecond.y);

                if(y + epsilon <= minAY || y + epsilon <= minBY){
                    return std::nullopt;
                }

                if(y - epsilon >= maxAY || y - epsilon >= maxBY){
                    return std::nullopt;
                }

                return Vec3(startFirst.x, y);

            }

            float fdy = endFirst.y - startFirst.y;
            float fdx = endFirst.x - startFirst.x;
            float fk = fdy/fdx;
            float fb = startFirst.y - fk * startFirst.x;
            float sdy = endSecond.y - startSecond.y;
            float sdx = endSecond.x - startSecond.x;
            float sk = sdy/sdx;
            float sb = startSecond.y - sk * startSecond.x;
            float x = (fb - sb) / (sk - fk);
            float y = fk * x + fb;

            float minAX = std::min(startFirst.x, endFirst.x);
            float minBX = std::min(startSecond.x, endSecond.x);
            float maxAX = std::max(startFirst.x, endFirst.x);
            float maxBX = std::max(startSecond.x, endSecond.x);

            if(x + epsilon <= minAX || x + epsilon <= minBX){
                return std::nullopt;
            }

            if(x - epsilon >= maxAX || x - epsilon >= maxBX){
                return std::nullopt;
            }

            return Vec3(x, y);
    }

    /**
    * Находит пересечение окружности и линии аналитическим способом (через решение системы уравнений)
    */
    std::vector<Vec3> findIntersectionLineAndCircle(
        const Vec3& lineFirstPoint,
        const Vec3& lineSecondPoint,
        const Vec3& circlePoint,
        float circleRadius,
        bool flag
    ) {
        long double dy = lineSecondPoint.y - lineFirstPoint.y;
        long double dx = lineSecondPoint.x - lineFirstPoint.x;
        long double k = dy/dx;
        long double b = lineFirstPoint.y - k * lineFirstPoint.x;
        long double y = circlePoint.y;
        long double x = circlePoint.x;
        long double r = circleRadius;
        std::vector<Vec3> result;

        bool isLineVertical = lineFirstPoint.x == lineSecondPoint.x;

        /**
        * Если линия вертикальная, то подставляем "x" в уравнение окружности и выводим "y"
        */
        if(isLineVertical){
            long double x1 = lineFirstPoint.x;
            long double y1 = std::sqrt(std::pow(r, 2.0L) - std::pow(x1 - x, 2.0L)) - y;
            long double minY = std::min(lineFirstPoint.y, lineSecondPoint.y);
            long double maxY = std::max(lineFirstPoint.y, lineSecondPoint.y);

            if(-y1 + epsilon >= minY){
                result.push_back(Vec3(x1, -y1, lineFirstPoint.z));
            }

            if(y1 - epsilon <= maxY){
                 result.push_back(Vec3(x1, y1, lineFirstPoint.z));
            }

            return result;
        }

        /**
        * Иначе решаем через систему уравнений
        */
        long double a = 1.0L + std::pow(k, 2.0L);
        long double db = 2.0f * (k * b - k * y - x);
        long double c = -(
            std::pow(r, 2.0L) -
            std::pow(x, 2.0L) -
            std::pow(b, 2.0L) +
            2.0L * y * b -
            std::pow(y, 2.0L)
        );

        long double d = std::pow(db, 2.0L) - 4.0L * a * c;

        /**
        * Если дискриминант меньше нуля, значит решений нет
        */
        if(d + epsilon < 0.0L){
            return result;
        }

        if(d < epsilon){
            d = 0.0L;
        }

        long double x1 = ( std::sqrt(d) - db) / (2.0L * a);
        long double x2 = (-std::sqrt(d) - db) / (2.0L * a);
        long double y1 = k * x1 + b;
        long double y2 = k * x2 + b;

        long double minY = std::min(y + r, y - r);
        long double maxY = std::max(y + r, y - r);
        long double minX = std::min(lineFirstPoint.x, lineSecondPoint.x);
        long double maxX = std::max(lineFirstPoint.x, lineSecondPoint.x);

        if(
            y1 - epsilon <= maxY &&
            y1 + epsilon >= minY &&
            x1 + epsilon >= minX &&
            x1 - epsilon <= maxX
        ){
            result.push_back(Vec3(x1, y1, lineFirstPoint.z));
        }

        if(
            y2 - epsilon <= maxY &&
            y2 + epsilon >= minY &&
            x2 + epsilon >= minX &&
            x2 - epsilon <= maxX
        ){
            result.push_back(Vec3(x2, y2, lineFirstPoint.z));
        }

        return result;
    }

    /**
    * Находит пересечние линии и ломанной состоящей из множества точек
    */
    std::vector<Vec3> findIntersectionLineAndPath(
        const Vec3& lineFirstPoint, 
        const Vec3& lineSecondPoint, 
        const std::vector<Vec3>& path){
        std::vector<Vec3> result;

        for(int i = 0; i < path.size() - 1; i++){
            auto point = findIntersectionLineAndLine(
                lineFirstPoint,
                lineSecondPoint,
                path[i],
                path[i + 1]
            );

            if(point){
                result.push_back(*point);
            }
        }
        return result;
    }


    /** 
    * Находит пересечение окружности и ломанной состоящей из множества точек
    */
    std::vector<Vec3> findIntersectionCircleAndPath(
        const Vec3& centerPoint, 
        float radius, 
        const std::vector<Vec3>& path){
        std::vector<Vec3> result;

        for(int i = 0; i < path.size() - 1; i++){
            auto points = findIntersectionLineAndCircle(
                path[i],
                path[i + 1],
                centerPoint,
                radius
            );

            if(points.size()){
                result.insert(result.begin(), points.begin(), points.end());
            }
        }
        return result;
    }

    /**
    * Получить угол по трём точкам; center - точка угла
    */
    float angleByPoints(const Vec3& center, const Vec3& a, const Vec3& b){
        return std::abs(
            radToDeg(
                std::atan2(b.y - center.y, b.x - center.x) -
                std::atan2(a.y - center.y, a.x - center.x)
        ));
    }

    /**
    * Построить перпендикуляр из точки point в отрезок a-b
     */
    Vec3 perpendicularToLine(const Vec3& point, const Vec3& a, const Vec3& b){
        float angle = 180.0f - 90.0f - angleByPoints(a, point, b);
        /** длина гиппотенузы */
        float cl = dist(a, point);
        /** длина катета лежащего на линии a - b */
        float bl = cl * std::sin(degToRad(angle));
        Vec3 h = movePointAtLineByScalar(a, b, bl);
        return h;
    }

    /**
    * Отражает точку симметрично относительно указанного отрезка lineFirstPoint-lineSecondPoint
    */
    Vec3 symmetryMirror(const Vec3& point, const Vec3& lineFirstPoint, const Vec3& lineSecondPoint){
        Vec3 h = perpendicularToLine(point, lineFirstPoint, lineSecondPoint);
        return movePointAtLineByPercent(point, h, 200);

    }
    
};