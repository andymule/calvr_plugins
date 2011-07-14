#ifndef _GREEN_LIGHT_H_
#define _GREEN_LIGHT_H_

#include <list>
#include <set>
#include <vector>

#include <osg/AnimationPath>
#include <config/ConfigManager.h>
#include <kernel/CVRPlugin.h>
#include <menu/MenuButton.h>
#include <menu/MenuCheckbox.h>
#include <menu/SubMenu.h>

#include <osg/MatrixTransform>

#include "Utility.h"

class GreenLight : public cvr::CVRPlugin, public cvr::MenuCallback
{
    public:
        GreenLight();
        ~GreenLight();

        bool init();

        void menuCallback(cvr::MenuItem * item);

        void preFrame();
        void postFrame();
        bool keyEvent(bool keyDown, int key, int mod);
        bool buttonEvent(int type, int button, int hand, const osg::Matrix& mat);
        bool mouseButtonEvent(int type, int button, int x, int y, const osg::Matrix& mat);

    protected:

        class Component; // forward declaration

        class Entity
        {
            public:
                enum AnimationStatus { START, FORWARD, END, REVERSE };

                Entity(osg::Node * node, osg::Matrix mat = osg::Matrix::identity());

                osg::ref_ptr<osg::MatrixTransform> transform; // transform nodes of this entity
                osg::ref_ptr<osg::AnimationPath> path; // animation path (null if non-existent)
                osg::ref_ptr<osg::Node> mainNode;
// TODO change nodes to type: set< ref_ptr< Node > >
                std::set<osg::Node *> nodes; // node-sub-graph loaded in via osgDB readNodeFile
                AnimationStatus status; // status of animation
                double time; // time-point within animation path
                std::list<Entity *> group; // other entities that should animate together

                void handleAnimation();
                void beginAnimation();
                void addChild(Entity * child);
                void showVisual(bool show);
                virtual void setTransparency(bool transparent);

                virtual Component * asComponent() {return NULL;}

            protected:
                void createNodeSet(osg::Node * node);
        };

        class Component : public Entity
        {
            public:
                Component(osg::Node * node, std::string componentName, osg::Matrix mat = osg::Matrix::identity());

                std::string name;
                bool selected;
                int minWattage; // used for coloring range
                int maxWattage; // used for coloring range
                std::string cluster;

                void setColor(const osg::Vec3 color);
                void setDefaultMaterial();
                void setTransparency(bool transparent);
                bool select(bool select);

                Component * asComponent() {return this;}
        };

        typedef struct {
            std::string name;
            int rack;
            int slot;
            int height;
         } Hardware;

        // Menu Items
        cvr::SubMenu * _glMenu;
        cvr::MenuCheckbox * _showSceneCheckbox;

        cvr::SubMenu * _hardwareSelectionMenu;
        cvr::MenuCheckbox * _selectionModeCheckbox;
        cvr::SubMenu * _selectClusterMenu;
        std::set< cvr::MenuCheckbox * > _clusterCheckbox;
        cvr::MenuButton * _selectAllButton;
        cvr::MenuButton * _deselectAllButton;

        cvr::SubMenu * _displayComponentsMenu;
        cvr::MenuCheckbox * _componentsViewCheckbox;
        cvr::MenuCheckbox * _displayFrameCheckbox;
        cvr::MenuCheckbox * _displayDoorsCheckbox;
        cvr::MenuCheckbox * _displayWaterPipesCheckbox;
        cvr::MenuCheckbox * _displayElectricalCheckbox;
        cvr::MenuCheckbox * _displayFansCheckbox;
        cvr::MenuCheckbox * _displayRacksCheckbox;

        cvr::SubMenu * _powerMenu;
        cvr::MenuCheckbox * _displayPowerCheckbox;
        cvr::MenuButton * _loadPowerButton;

        // Entities
        Entity * _box;          // box/frame
        std::vector<Entity *> _door; // doors
        Entity * _waterPipes;   // water pipes
        Entity * _electrical;   // electrical
        Entity * _fans;         // fans
        std::vector<Entity *> _rack; // racks
        std::set<Component *> _components; // components in the racks

        // Additional Entity Info
        Entity * _mouseOver;
        Entity * _wandOver;
        std::map< std::string, std::set< Component * > * > _cluster;

        // File contents -- read/write via master node, copy to slave nodes via messages
        std::string _hardwareContents;
        std::string _powerContents;

        // Functions
        bool loadScene();
        bool handleIntersection(osg::Node * iNode);
        void parseHardwareFile();
        void setPowerColors(bool displayPower);
        void selectComponent(Component * comp, bool select);
        void selectCluster(std::set< Component * > * cluster, bool select);
        void handleHoverOver(osg::Matrix pointerMat, Entity *& hovered);
        void doHoverOver(Entity *& last, Entity * current);
};

#endif
