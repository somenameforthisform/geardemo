#include <variant>
#include <optional>
#include <array>
#include <functional>

enum FigureTypes {Point, Line, Circle, Vector};

class SceneFigure {
    public:
    FigureTypes type;
    std::array<unsigned char, 4> color;
    bool hidden = false;

    SceneFigure(FigureTypes typeargs, std::array<unsigned char, 4> color = std::array<unsigned char, 4>{255,0,0,255}): type{typeargs}, color{color} {};
    virtual ~SceneFigure() = default;
};

class FigureLine: public SceneFigure {
    public: 
    Vec3 first, second;
    FigureLine(Vec3 f, Vec3 s, std::array<unsigned char, 4> color = std::array<unsigned char, 4>{255,0,0,255}): first{f}, second{s}, SceneFigure(FigureTypes::Line, color){};
};

class FigureCircle: public SceneFigure { 
    public:
    Vec3 center;
    float radius;
    FigureCircle(Vec3 c, float r, std::array<unsigned char, 4> color = std::array<unsigned char, 4>{255,0,0,255}): center{c}, radius{r}, SceneFigure(FigureTypes::Circle, color){};
};

class FigurePoint: public SceneFigure { 
    public:
    Vec3 pos;
    FigurePoint(Vec3 c, std::array<unsigned char, 4> color = std::array<unsigned char, 4>{255,0,0,255}): pos{c}, SceneFigure(FigureTypes::Point, color){};
};


class Scene {
    public:
    int currentAnimationStep = 0;
    bool animationStarted = false;
    std::vector<std::unique_ptr<SceneFigure>> items;
    std::vector<std::function<bool(Scene&)>> animations;

    template <typename T> requires 
    std::is_same<T, FigureLine>::value   || 
    std::is_same<T, FigureCircle>::value || 
    std::is_same<T, FigurePoint>::value
    int push(T item){
        items.push_back(std::make_unique<T>(item));
        return (items.size() - 1);
    }

    template <typename T> requires 
    std::is_same<T, FigureLine>::value   || 
    std::is_same<T, FigureCircle>::value || 
    std::is_same<T, FigurePoint>::value
    T* get(int idx){
        return static_cast<T*>(items[idx].get());
    }

    /**
     принимает в качестве аргумента callback, который будет вызываться на каждый кадр
     если callback возвращает true, то происходит переход на следующий шаг анимации
    */
    void addAnimationStep(std::function<bool(Scene&)> lambda){
        animations.push_back(lambda);
    }

    void runAnimation(){
        animationStarted = true;
        if(currentAnimationStep < animations.size()){
            bool toNextStep = animations[currentAnimationStep](*this);
            if(toNextStep){
                currentAnimationStep++;
                animationStarted = false;
            }
        }
    }

    std::vector<Vec3> getPoints(){
        std::vector<Vec3> result;

        for(int i = 0; i < items.size(); i++){
            SceneFigure* figure = items[i].get();
            if(figure->type == FigureTypes::Point){
                auto figPoint = static_cast<FigurePoint*>(figure);
                Vec3 point(figPoint->pos.x, figPoint->pos.y);
                result.push_back(point);
            }
        }

        return result;
    }
};

