namespace Logic {

    struct BodyID;

struct Body {
    
};

struct PhysicsEngine {
    struct Limit {
        f32 min, max;


        bool operator< (Limit &other) const {
            return min < other.min;
        }
    };

    u32 num_bodies;
    Body *bodies;
};

};
