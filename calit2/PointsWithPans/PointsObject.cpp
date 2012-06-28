#include "PointsObject.h"
#include "PanMarkerObject.h"

#include <cvrKernel/PluginHelper.h>
#include <cvrConfig/ConfigManager.h>
#include <PluginMessageType.h>

using namespace cvr;

PointsObject::PointsObject(std::string name, bool navigation, bool movable, bool clip, bool contextMenu, bool showBounds) : SceneObject(name,navigation,movable,clip,contextMenu,showBounds)
{
    _activePanMarker = NULL;
    _transitionActive = false;
    _fadeActive = false;
    _transitionTime = 4.0;

    _totalFadeTime = 5.0;

    if(contextMenu)
    {
	_alphaRV = new MenuRangeValue("Alpha",0.0,1.0,1.0);
	_alphaRV->setCallback(this);
	addMenuItem(_alphaRV);
    }
    else
    {
	_alphaRV = NULL;
    }

    _root->getOrCreateStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
    std::string bname = "Points";
    _root->getOrCreateStateSet()->setRenderBinDetails(1,bname);
}

PointsObject::~PointsObject()
{

}

bool PointsObject::getPanActive()
{
    return _activePanMarker;
}

void PointsObject::setActiveMarker(PanMarkerObject * marker)
{
    _activePanMarker = marker;
    if(marker)
    {
	startTransition();
    }
}

void PointsObject::panUnloaded()
{
    if(_activePanMarker)
    {
	//_root->setNodeMask(_storedNodeMask);
	if(_alphaUni)
	{
	    _alphaUni->set(1.0f);
	}
	attachToScene();
	_activePanMarker->panUnloaded();
	_activePanMarker = NULL;
	setNavigationOn(true);

	for(int i = 0; i < getNumChildObjects(); i++)
	{
	    PanMarkerObject * pmo = dynamic_cast<PanMarkerObject*>(getChildObject(i));
	    if(pmo)
	    {
		pmo->unhide();
	    }
	}
    }
}

void PointsObject::clear()
{
    _root->removeChildren(0,_root->getNumChildren());
    setTransform(osg::Matrix::identity());
    _alphaUni = NULL;
}

void PointsObject::setAlpha(float alpha)
{
    if(!_alphaUni)
    {
	for(int i = 0; i < getNumChildNodes(); i++)
	{
	    _alphaUni = getChildNode(i)->getOrCreateStateSet()->getUniform("globalAlpha");
	    if(_alphaUni)
	    {
		break;
	    }
	}
    }

    if(_alphaUni)
    {
	_alphaUni->set(alpha);
    }

    if(_alphaRV)
    {
	_alphaRV->setValue(alpha);
    }

}

float PointsObject::getAlpha()
{
    if(!_alphaUni)
    {
	for(int i = 0; i < getNumChildNodes(); i++)
	{
	    _alphaUni = getChildNode(i)->getOrCreateStateSet()->getUniform("globalAlpha");
	    if(_alphaUni)
	    {
		break;
	    }
	}
    }

    if(_alphaUni)
    {
	float a;
	_alphaUni->get(a);
	return a;
    }

    return 1.0;
}

void PointsObject::update()
{
    if(_transitionActive)
    {
	float lastStatus = _transition / _transitionTime;
	_transition += PluginHelper::getLastFrameDuration();
	if(_transition > _transitionTime)
	{
	    _transition = _transitionTime;
	}

	float status = _transition / _transitionTime;
	status -= lastStatus;

	osg::Vec3 movement = (_endCenter - _startCenter);
	movement.x() *= status;
	movement.y() *= status;
	movement.z() *= status;
	osg::Matrix m;
	m.makeTranslate(movement);

	setTransform(getTransform() * m);

	if(_transition == _transitionTime)
	{
	    _transitionActive = false;
	    if(_activePanMarker->loadPan())
	    {
		startFade();
		//_storedNodeMask = _root->getNodeMask();
		//_root->setNodeMask(0);
	    }
	    else
	    {
		_activePanMarker = NULL;
		setNavigationOn(true);
	    }
	}
    }
    else if(_fadeActive)
    {
	if(_skipFrames > 0)
	{
	    _skipFrames--;
	}
	else
	{
	    _fadeTime += PluginHelper::getLastFrameDuration();
	    if(_fadeTime > _totalFadeTime)
	    {
		_fadeTime = _totalFadeTime;
	    }

	    setAlpha(1.0f - (_fadeTime / _totalFadeTime));
	    float panAlpha = _fadeTime / _totalFadeTime;
	    PluginHelper::sendMessageByName("PanoViewLOD",PAN_SET_ALPHA,(char*)&panAlpha);

	    if(_fadeTime == _totalFadeTime)
	    {
		_fadeActive = false;
		detachFromScene();
		
	    }
	}
    }
}

void PointsObject::updateCallback(int handID, const osg::Matrix & mat)
{
    osg::Vec3 viewerPos = PluginHelper::getHeadMat().getTrans();
    for(int i = 0; i < getNumChildObjects(); i++)
    {
	osg::Vec3 spherePos = getChildObject(i)->getObjectToWorldMatrix().getTrans();
	float distance = (spherePos - viewerPos).length();
	PanMarkerObject * pmo = dynamic_cast<PanMarkerObject*>(getChildObject(i));
	if(pmo)
	{
	    pmo->setViewerDistance(distance);
	}
    }
}

void PointsObject::menuCallback(MenuItem * item)
{
    if(_alphaRV && item == _alphaRV)
    {
	if(!_alphaUni)
	{
	    for(int i = 0; i < getNumChildNodes(); i++)
	    {
		_alphaUni = getChildNode(i)->getOrCreateStateSet()->getUniform("globalAlpha");
		if(_alphaUni)
		{
		    break;
		}
	    }
	}

	if(_alphaUni)
	{
	    _alphaUni->set(_alphaRV->getValue());
	}
    }

    SceneObject::menuCallback(item);
}

void PointsObject::startTransition()
{
    //TODO: find to/from movement points
    _transitionActive = true;
    setNavigationOn(false);
    _transition = 0.0;

    osg::Vec3 offset = ConfigManager::getVec3("Plugin.PanoViewLOD.Offset");
    offset = offset + osg::Vec3(0,0,_activePanMarker->getCenterHeight());

    _endCenter = offset;
    _startCenter = _activePanMarker->getObjectToWorldMatrix().getTrans();

    for(int i = 0; i < getNumChildObjects(); i++)
    {
	PanMarkerObject * pmo = dynamic_cast<PanMarkerObject*>(getChildObject(i));
	if(pmo)
	{
	    pmo->hide();
	}
    }

}

void PointsObject::startFade()
{
    _fadeActive = true;

    setAlpha(1.0);

    float panAlpha = 0.0;
    PluginHelper::sendMessageByName("PanoViewLOD",PAN_SET_ALPHA,(char*)&panAlpha);

    _fadeTime = 0;
    _skipFrames = 3;
}