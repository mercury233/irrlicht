// Copyright (C) 2008-2012 Colin MacDonald
// No rights reserved: this software is in the public domain.

#include "testUtils.h"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

static void setupTestScene(IVideoDriver* driver, ISceneManager * smgr, IGUIEnvironment* env, ITexture* testTexture)
{
	env->addCheckBox(true, core::recti(10,60,28,82));

	IBillboardSceneNode * bnode = smgr->addBillboardSceneNode(0,dimension2d<f32>(32,32),core::vector3df(0,0,10));
	bnode->setMaterialFlag(video::EMF_LIGHTING, false);
	bnode->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR);
	bnode->setMaterialTexture(0, testTexture);

	smgr->addTextSceneNode(env->getBuiltInFont(), L"TEST", video::SColor(255,255,255,255), 0);
	// Lighting off and vertex color to black to unify results a bit. Was with lighting in the past which made cube black on all but software driver.
	irr::scene::IMeshSceneNode* cubeNode = smgr->addCubeSceneNode();
	cubeNode->setMaterialFlag(video::EMF_LIGHTING, false);
	smgr->getMeshManipulator()->setVertexColors(cubeNode->getMesh(), irr::video::SColor(255, 0,0,0));
	smgr->addCameraSceneNode(0, vector3df(0,30,-40), vector3df(0,5,0));
}

//! Tests view ports and text rendering.
/** The result should be as follows: We have three times the same image, a billboard with a cube and the text in front.
They are replicated three times, one centered in the screen without viewport, one in the upper left (qurter screen) and
one on the right (half screen). The latter is stretched due to the changed aspect ratio.
The two viewport scenes get the texture drawn over the center as well, using draw2dimage. This should mark the proper
image position with the top left corner of the texture.
Finally, each scene has a checkbox drawn at the left side, vertically centered. This will show whether GUI elements adhere
to viewports as well. */
static bool viewPortText(E_DRIVER_TYPE driverType)
{
	dimension2du wndSize(160, 120);
	IrrlichtDevice *device = createDevice( driverType, wndSize, 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();
	IGUIEnvironment* env = smgr->getGUIEnvironment();

	stabilizeScreenBackground(driver);
	logTestString("Testing driver %ls\n", driver->getName());

	setupTestScene(driver, smgr, env, driver->getTexture("../media/fire.bmp"));

	driver->beginScene(video::ECBF_COLOR | video::ECBF_DEPTH, SColor(255,100,101,140));
	smgr->drawAll();
	env->drawAll();
	driver->setViewPort(rect<s32>(0,0,wndSize.Width/2,wndSize.Height/2));
	smgr->drawAll();
	env->drawAll();
	driver->draw2DImage(driver->getTexture("../media/fire.bmp"), core::vector2di(wndSize.Width/2,wndSize.Height/2));
	driver->setViewPort(rect<s32>(wndSize.Width/2,0,wndSize.Width,wndSize.Height));
	smgr->drawAll();
	env->drawAll();
	driver->draw2DImage(driver->getTexture("../media/fire.bmp"), core::vector2di(wndSize.Width/2,wndSize.Height/2));
	driver->endScene(); 

	bool result = takeScreenshotAndCompareAgainstReference(driver, "-viewPortText.png", 98.71f);

	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

//! Test view ports with render target textures
static bool viewPortRTTs(E_DRIVER_TYPE driverType)
{
	dimension2du wndSize(160, 120);
	IrrlichtDevice *device = createDevice( driverType, wndSize, 32);
	if (!device)
		return true; // Treat a failure to create a driver as benign; this saves a lot of #ifdefs

	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager * smgr = device->getSceneManager();
	IGUIEnvironment* env = smgr->getGUIEnvironment();

	stabilizeScreenBackground(driver);
	logTestString("Testing driver %ls\n", driver->getName());

	video::ITexture* testTexture = driver->getTexture("../media/fire.bmp");
	if ( !testTexture )
	{
		logTestString("Missing test texture ../media/fire.bmp\n");
		return false;
	}
	IGUIFont* font = env->getSkin()->getFont();
	if ( !font )
	{
		logTestString("Missing font\n");
		return false;
	}
	irr::core::stringw testStr(L"BoBoBoBoBoBoBoBoBoBo");
	core::recti rectText(20, 20, 120, 40);
	SColor textCol(255, 255, 240, 255);
	dimension2du rttSize(64, 72); // height to cut ui element in half 
	core::recti rectTexDest(rttSize.Width-16,4,rttSize.Width+16, 36);

	setupTestScene(driver, smgr, env, testTexture);
	
	video::ITexture* rtt = driver->addRenderTargetTexture(rttSize, "rt", video::ECF_A8R8G8B8);
	if ( !rtt )
	{
		logTestString("Driver can't create rtt, test ignored\n");
		return true;
	}

	logTestString("rtt size wanted: %u %u real: %u %u\n", rttSize.Width, rttSize.Height, rtt->getSize().Width, rtt->getSize().Height);
	core::recti vp = driver->getViewPort();
	logTestString("vp original: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);

	driver->beginScene(video::ECBF_COLOR | video::ECBF_DEPTH, SColor(255,100,101,240));
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();

#if 1 // rendertarget - should set it's own viewport, but scaling
	driver->setRenderTarget(rtt, ECBF_ALL, SColor(255, 150, 120, 120));

	vp = driver->getViewPort();
	logTestString("vp after setting rtt: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();
	driver->setRenderTarget(0, 0);

	vp = driver->getViewPort();
	logTestString("vp after resetting rtt: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);

	driver->draw2DImage(rtt, vector2di(2,2), true);
#endif

#if 1 // changing view-port just to see if RT will still reset it correctly when it's not screen-size
	driver->setViewPort(rect<s32>(0,0,wndSize.Width/2,wndSize.Height/2));
	vp = driver->getViewPort();
	logTestString("vp set outside rtt: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
#endif

#if 1 // rendertarget with viewport in it's left upper quarter
	driver->setRenderTarget(rtt, ECBF_ALL, SColor(255, 120, 120, 150));
	vp = driver->getViewPort();
	logTestString("vp setting rtt again: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);

	driver->setViewPort(rect<s32>(0,0,rttSize.Width/2,rttSize.Height/2));
	vp = driver->getViewPort();
	logTestString("vp set inside rtt: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();

	driver->setRenderTarget(0, 0);
	driver->draw2DImage(rtt, core::vector2di(wndSize.Width-rttSize.Width-2,2), true);
#endif


#if 1 // rendertarget with viewport in it's right lower quarter, then upper left quarter, then 2 times partly outside with non-uniform scaling
	driver->setRenderTarget(rtt, ECBF_ALL, SColor(255, 120, 150, 120));

	driver->setViewPort(rect<s32>(rttSize.Width/2,rttSize.Height/2,rttSize.Width,rttSize.Height));
	vp = driver->getViewPort();
	logTestString("vp rtt lower-right: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();

	driver->setViewPort(rect<s32>(0,0,rttSize.Width/2,rttSize.Height/2));
	vp = driver->getViewPort();
	logTestString("vp rtt upper-left: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();

	bool clipToRt = driverType != EDT_OPENGL; // opengl is the only driver so far supporting unclipped vp
	driver->setViewPort(rect<s32>(rttSize.Width-rttSize.Width/4,-(s32)rttSize.Height/2,rttSize.Width+rttSize.Width/4,rttSize.Height/2), clipToRt);
	vp = driver->getViewPort();
	logTestString("vp rtt beyond uppper-right: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();
	
	driver->setViewPort(rect<s32>(-(s32)rttSize.Width/2,rttSize.Height-rttSize.Height/4,rttSize.Width/2,rttSize.Height+rttSize.Height/4), clipToRt);
	vp = driver->getViewPort();
	logTestString("vp beyond rtt lower-left: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);
	smgr->drawAll();
	driver->draw2DImage(testTexture, rectTexDest, core::recti(testTexture->getSize()));
	font->draw(testStr, rectText, textCol);
	env->drawAll();


	driver->setRenderTarget(0, 0);
	driver->draw2DImage(rtt, core::vector2di(wndSize.Width-rttSize.Width-2,wndSize.Height-rttSize.Height-2), true);
#endif

	driver->endScene(); 
	vp = driver->getViewPort();
	logTestString("vp after endScene: %d %d %d %d\n", vp.UpperLeftCorner.X, vp.UpperLeftCorner.Y, vp.LowerRightCorner.X, vp.LowerRightCorner.Y);


	bool result = takeScreenshotAndCompareAgainstReference(driver, "-viewPortRTT.png", 99.9f);
	
	device->closeDevice();
	device->run();
	device->drop();

	return result;
}

bool viewPort(void)
{
	bool result = true;

	// TODO: software driver doesn't t use view port for 2d rendering, so result is pretty wrong.
	TestWithAllDrivers(viewPortText);
	//TestWithAllDrivers(viewPortRTTs); // not yet working well with software drivers
	TestWithAllHWDrivers(viewPortRTTs);

	return result;
}

