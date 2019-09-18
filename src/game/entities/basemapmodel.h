#ifndef BASEMAPMODEL_H
#define BASEMAPMODEL_H

namespace entities {
    namespace classes {

        // The base entity class for any model. (This could be a tree, or even doors and other interactive items.)
        // NOTE: Never change its type, it is MAPMODEL for a reason.
        class BaseMapModel : public BaseEntity {
        public:
            BaseMapModel(const std::string &filename);
            virtual ~BaseMapModel();

            virtual void preload();
            virtual void think();
            virtual void render();

            virtual void onAttributeSet(const std::string &key, const std::string &value);

        private:
            void loadModelAttributes();

        public:
            void preloadMapModel(const std::string &filename);
        };
    } // classes
} // entities

#endif // BASEMAPMODEL_H
