#ifndef PLUGIN_TEST
#define PLUGIN_TEST

#include <kernel/CVRPlugin.h>
#include <kernel/ScreenMVSimulator.h>
#include <menu/MenuButton.h>
#include <menu/SubMenu.h>
#include <menu/MenuCheckbox.h>
#include <menu/MenuRangeValue.h>
#include <menu/PopupMenu.h>

class SMV2Settings : public cvr::CVRPlugin, public cvr::MenuCallback
{
    public:
        SMV2Settings();
        ~SMV2Settings();

        bool init();

        void menuCallback(cvr::MenuItem * item);

    protected:
        cvr::SubMenu * mvsMenu;
        cvr::MenuCheckbox * multipleUsers;
        cvr::SubMenu * contributionMenu;
        cvr::MenuCheckbox * linearFunc;
        cvr::MenuCheckbox * gaussianFunc;
        cvr::MenuCheckbox * orientation3d;
        cvr::SubMenu * zoneMenu;
        cvr::MenuCheckbox * autoAdjust;
        cvr::MenuRangeValue * zoneRowQuantity;
        cvr::MenuRangeValue * zoneColumnQuantity;
        cvr::MenuRangeValue * autoAdjustTarget;
        cvr::MenuRangeValue * autoAdjustOffset;
};

#endif
