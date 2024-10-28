//==============================================================================
/*
    Software License Agreement (BSD License)
    Copyright (c) 2003-2016, CHAI3D.
    (www.chai3d.org)

    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    * Neither the name of CHAI3D nor the names of its contributors may
    be used to endorse or promote products derived from this software
    without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE. 

    \author    <http://www.chai3d.org>
    \author    Francois Conti
    \version   3.2.0 $Rev: 2049 $
*/
//==============================================================================

//------------------------------------------------------------------------------
#include "chai3d.h"
#include "bottle.h"
#include <mutex>
//------------------------------------------------------------------------------
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>
//------------------------------------------------------------------------------
using namespace chai3d;
using namespace std;
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// GENERAL SETTINGS
//------------------------------------------------------------------------------

// stereo Mode
/*
    C_STEREO_DISABLED:            Stereo is disabled 
    C_STEREO_ACTIVE:              Active stereo for OpenGL NVDIA QUADRO cards
    C_STEREO_PASSIVE_LEFT_RIGHT:  Passive stereo where L/R images are rendered next to each other
    C_STEREO_PASSIVE_TOP_BOTTOM:  Passive stereo where L/R images are rendered above each other
*/
cStereoMode stereoMode = C_STEREO_DISABLED;

// fullscreen mode
bool fullscreen = false;

// mirrored display
bool mirroredDisplay = false;


//------------------------------------------------------------------------------
// DECLARED VARIABLES
//------------------------------------------------------------------------------

// a world that contains all objects of the virtual environment
cWorld* world;

// a camera to render the world in the window display
cCamera* camera;

// a light source to illuminate the objects in the world
cDirectionalLight *light;

// a virtual object
cMultiMesh* object;


// water_bottle object
cShapeCylinder* water_bottle;
bottle* w_bottle;
cMultiMesh* cup;
cShapeSphere* com_ball;
cMultiMesh* arrow;
cShapeLine* surface_line;

// a haptic device handler
cHapticDeviceHandler* handler;

// a pointer to the current haptic device
cGenericHapticDevicePtr hapticDevice;

// a virtual tool representing the haptic device in the scene
cToolCursor* tool;

// a colored background
cBackground* background;

// a font for rendering text
cFontPtr font;

// a label to display the rate [Hz] at which the simulation is running
cLabel* labelRates;

// a flag that indicates if the haptic simulation is currently running
bool simulationRunning = false;

// a flag that indicates if the haptic simulation has terminated
bool simulationFinished = true;

// display options
bool showEdges = true;
bool showTriangles = true;
bool showNormals = false;

// display level for collision tree
int collisionTreeDisplayLevel = 0;

// a frequency counter to measure the simulation graphic rate
cFrequencyCounter freqCounterGraphics;

// a frequency counter to measure the simulation haptic rate
cFrequencyCounter freqCounterHaptics;

// haptic thread
cThread* hapticsThread;

// a handle to window display context
GLFWwindow* window = NULL;

// current width of window
int width  = 0;

// current height of window
int height = 0;

// swap interval for the display context (vertical synchronization)
int swapInterval = 1;

// root resource path
string resourceRoot;





// Center of Mass tester
double cx;
double cy;





//------------------------------------------------------------------------------
// DECLARED MACROS
//------------------------------------------------------------------------------

// convert to resource path
#define RESOURCE_PATH(p)    (char*)((resourceRoot+string(p)).c_str())


//------------------------------------------------------------------------------
// DECLARED FUNCTIONS
//------------------------------------------------------------------------------

// callback when the window display is resized
void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height);

// callback when an error GLFW occurs
void errorCallback(int error, const char* a_description);

// callback when a key is pressed
void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods);

// callback to render graphic scene
void updateGraphics(void);

// this function renders the scene
void updateGraphics(void);

// this function contains the main haptics simulation loop
void updateHaptics(void);

// this function closes the application
void close(void);


//==============================================================================
/*
    DEMO:   21-object.cpp

    This demonstration loads a 3D mesh file by using the file loader
    functionality of the \ref cMultiMesh class. A finger-proxy algorithm is
    used to render the forces. Take a look at this example to understand the
    different functionalities offered by the tool force renderer.

    In the main haptics loop function  "updateHaptics()" , the position
    of the haptic device is retrieved at each simulation iteration.
    The interaction forces are then computed and sent to the device.
    Finally, a simple dynamics model is used to simulate the behavior
    of the object.
*/
//==============================================================================

int main(int argc, char* argv[])
{
    //--------------------------------------------------------------------------
    // INITIALIZATION
    //--------------------------------------------------------------------------

    cout << endl;
    cout << "-----------------------------------" << endl;
    cout << "CHAI3D" << endl;
    cout << "Demo: 21-object" << endl;
    cout << "Copyright 2003-2016" << endl;
    cout << "-----------------------------------" << endl << endl << endl;
    cout << "Keyboard Options:" << endl << endl;
    cout << "[1] - Texture   (ON/OFF)" << endl;
    cout << "[2] - Wireframe (ON/OFF)" << endl;
    cout << "[3] - Collision tree (ON/OFF)" << endl;
    cout << "[4] - Increase collision tree display depth" << endl;
    cout << "[5] - Decrease collision tree display depth" << endl;
    cout << "[s] - Save screenshot to file" << endl;
    cout << "[e] - Enable/Disable display of edges" << endl;
    cout << "[t] - Enable/Disable display of triangles" << endl;
    cout << "[n] - Enable/Disable display of normals" << endl;
    cout << "[f] - Enable/Disable full screen mode" << endl;
    cout << "[m] - Enable/Disable vertical mirroring" << endl;
    cout << "[q] - Exit application" << endl;


	cout << "[a] - Add water to bottle" << endl;
	cout << "[b] - Remove water from bottle" << endl;
	cout << "[c] - Stop water add/remove" << endl;
    cout << endl << endl;

    // parse first arg to try and locate resources
    resourceRoot = string(argv[0]).substr(0,string(argv[0]).find_last_of("/\\")+1);


    //--------------------------------------------------------------------------
    // OPEN GL - WINDOW DISPLAY
    //--------------------------------------------------------------------------

    // initialize GLFW library
    if (!glfwInit())
    {
        cout << "failed initialization" << endl;
        cSleepMs(1000);
        return 1;
    }

    // set error callback
    glfwSetErrorCallback(errorCallback);

    // compute desired size of window
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    int w = 0.8 * mode->height;
    int h = 0.5 * mode->height;
    int x = 0.5 * (mode->width - w);
    int y = 0.5 * (mode->height - h);

    // set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

    // set active stereo mode
    if (stereoMode == C_STEREO_ACTIVE)
    {
        glfwWindowHint(GLFW_STEREO, GL_TRUE);
    }
    else
    {
        glfwWindowHint(GLFW_STEREO, GL_FALSE);
    }

    // create display context
    window = glfwCreateWindow(w, h, "CHAI3D", NULL, NULL);
    if (!window)
    {
        cout << "failed to create window" << endl;
        cSleepMs(1000);
        glfwTerminate();
        return 1;
    }

    // get width and height of window
    glfwGetWindowSize(window, &width, &height);

    // set position of window
    glfwSetWindowPos(window, x, y);

    // set key callback
    glfwSetKeyCallback(window, keyCallback);

    // set resize callback
    glfwSetWindowSizeCallback(window, windowSizeCallback);

    // set current display context
    glfwMakeContextCurrent(window);

    // sets the swap interval for the current display context
    glfwSwapInterval(swapInterval);

#ifdef GLEW_VERSION
    // initialize GLEW library
    if (glewInit() != GLEW_OK)
    {
        cout << "failed to initialize GLEW library" << endl;
        glfwTerminate();
        return 1;
    }
#endif


    //--------------------------------------------------------------------------
    // WORLD - CAMERA - LIGHTING
    //--------------------------------------------------------------------------

    // create a new world.
    world = new cWorld();

    // set the background color of the environment
    world->m_backgroundColor.setBlack();

    // create a camera and insert it into the virtual world
    camera = new cCamera(world);
    world->addChild(camera);

    // define a basis in spherical coordinates for the camera
    camera->setSphericalReferences(cVector3d(0,0,0),    // origin
                                   cVector3d(0,0,1),    // zenith direction
                                   cVector3d(1,0,0));   // azimuth direction

    camera->setSphericalDeg(1.0,    // spherical coordinate radius
                            65,     // spherical coordinate polar angle
                            20);    // spherical coordinate azimuth angle

    // set the near and far clipping planes of the camera
    // anything in front or behind these clipping planes will not be rendered
    camera->setClippingPlanes(0.01, 100);

    // set stereo mode
    camera->setStereoMode(stereoMode);

    // set stereo eye separation and focal length (applies only if stereo is enabled)
    camera->setStereoEyeSeparation(0.03);
    camera->setStereoFocalLength(1.5);

    // set vertical mirrored display mode
    camera->setMirrorVertical(mirroredDisplay);

    // enable multi-pass rendering to handle transparent objects
    camera->setUseMultipassTransparency(true);

    // create a light source
    light = new cDirectionalLight(world);

    // attach light to camera
    camera->addChild(light);

    // enable light source
    light->setEnabled(true);

    // define the direction of the light beam
    light->setDir(-3.0,-0.5, 0.0);

    // set lighting conditions
    light->m_ambient.set(0.4f, 0.4f, 0.4f);
    light->m_diffuse.set(0.8f, 0.8f, 0.8f);
    light->m_specular.set(1.0f, 1.0f, 1.0f);


    //--------------------------------------------------------------------------
    // HAPTIC DEVICES / TOOLS
    //--------------------------------------------------------------------------

    // create a haptic device handler
    handler = new cHapticDeviceHandler();

    // get access to the first available haptic device found
    handler->getDevice(hapticDevice, 0);

    // retrieve information about the current haptic device
    cHapticDeviceInfo hapticDeviceInfo = hapticDevice->getSpecifications();

    // create a tool (cursor) and insert into the world
    tool = new cToolCursor(world);
    world->addChild(tool);

    // connect the haptic device to the virtual tool
    tool->setHapticDevice(hapticDevice);

    // if the haptic device has a gripper, enable it as a user switch
    hapticDevice->setEnableGripperUserSwitch(true);

    // define the radius of the tool (sphere)
    double toolRadius = 0.01;

    // define a radius for the tool
    tool->setRadius(toolRadius);

    // hide the device sphere. only show proxy.
    /*tool->setShowContactPoints(true, false);*/
	tool->setShowContactPoints(false, false);

    // create a white cursor
    /*tool->m_hapticPoint->m_sphereProxy->m_material->setWhite();*/

    // map the physical workspace of the haptic device to a larger virtual workspace.
    tool->setWorkspaceRadius(1.0);

    // oriente tool with camera
    tool->setLocalRot(camera->getLocalRot());

    // haptic forces are enabled only if small forces are first sent to the device;
    // this mode avoids the force spike that occurs when the application starts when 
    // the tool is located inside an object for instance. 
    tool->setWaitForSmallForce(true);

    // start the haptic tool
    tool->start();



	//--------------------------------------------------------------------------
	// CREATE Water_bottle and attach to tool
	//--------------------------------------------------------------------------
	water_bottle = new cShapeCylinder(0.07, 0.07, 0.3);
	//tool->m_image = water_bottle;


	cup = new cMultiMesh();
	
	bool fileload;
	fileload = cup->loadFromFile(RESOURCE_PATH("../resources/models/cup/Coffee_Mug.3ds"));

	if (!fileload)
	{
#if defined(_MSVC)
		fileload = cup->loadFromFile("../../../bin/resources/models/cup/Coffee_Mug.3ds");
#endif
	}
	if (!fileload)
	{
		cout << "Error - 3D Model failed to load correctly." << endl;
		close();
		return (-1);
	}

	// disable culling so that faces are rendered on both sides
	cup->setUseCulling(false);

	// scale model
	cup->scale(0.05);

	// use display list for faster rendering
	cup->setUseDisplayList(true);

	// position object in scene
	cup->rotateExtrinsicEulerAnglesDeg(0, 0, 0, C_EULER_ORDER_XYZ);
	cup->setUseTransparency(false);

	cup->setTransparencyLevel(0.3);

	tool->m_image = cup;
	tool->m_image->rotateExtrinsicEulerAnglesDeg(90, 0, 0, C_EULER_ORDER_XYZ);

	w_bottle = new bottle(0.12, 0.3);


	arrow = new cMultiMesh();
	fileload = arrow->loadFromFile(RESOURCE_PATH("../resources/models/arrow/arrow.obj"));

	if (!fileload)
	{
#if defined(_MSVC)
		fileload = arrow->loadFromFile("../../../bin/resources/models/arrow/arrow.obj");
#endif
	}
	if (!fileload)
	{
		cout << "Error - 3D Model failed to load correctly." << endl;
		close();
		return (-1);
	}


	// disable culling so that faces are rendered on both sides
	arrow->setUseCulling(false);

	cColorf color_arrow;
	color_arrow.setRed();
	arrow->setUseVertexColors(true);
	arrow->setVertexColor(color_arrow);

	// scale model
	

	// use display list for faster rendering
	arrow->setUseDisplayList(true);

	// position object in scene
	arrow->rotateExtrinsicEulerAnglesDeg(0, 90, 0, C_EULER_ORDER_XYZ);

	arrow->scaleXYZ(0.2,0.05,0.05);
	
	


	world->addChild(arrow);

    //--------------------------------------------------------------------------
    // CREATE WATER_TAP
    //--------------------------------------------------------------------------

    // read the scale factor between the physical workspace of the haptic
    // device and the virtual workspace defined for the tool
    double workspaceScaleFactor = tool->getWorkspaceScaleFactor();

    // stiffness properties
    double maxStiffness	= hapticDeviceInfo.m_maxLinearStiffness / workspaceScaleFactor;

    // create a virtual mesh
    object = new cMultiMesh();

    // add object to world
    world->addChild(object);

    // load an object file
    
    //fileload = object->loadFromFile(RESOURCE_PATH("../resources/models/leica/leica.3ds"));
    //if (!fileload)
    //{
    //    #if defined(_MSVC)
    //    fileload = object->loadFromFile("../../../bin/resources/models/leica/leica.3ds");
    //    #endif
    //}
	fileload = object->loadFromFile(RESOURCE_PATH("../resources/models/water_tap/faucet.3ds"));
	if (!fileload)
	{
		#if defined(_MSVC)
		fileload = object->loadFromFile("../../../bin/resources/models/water_tap/faucet.3ds");
		#endif
	}
    if (!fileload)
    {
        cout << "error - 3d model failed to load correctly" << endl;
        close();
        return (-1);
    }

    // disable culling so that faces are rendered on both sides
    object->setUseCulling(false);

    // get dimensions of object
    object->computeBoundaryBox(true);
    double size = cSub(object->getBoundaryMax(), object->getBoundaryMin()).length();

    // resize object to screen
    if (size > 0.001)
    {
        object->scale(1.0 / size);
    }

	object->scale(0.3);
     // compute a boundary box
    object->computeBoundaryBox(true);

    // show/hide boundary box
    object->setShowBoundaryBox(false);

    // compute collision detection algorithm
    object->createAABBCollisionDetector(toolRadius);

    // define a default stiffness for the object
    object->setStiffness(0.2 * maxStiffness, true);

    // define some haptic friction properties
    object->setFriction(0.1, 0.2, true);

    // enable display list for faster graphic rendering
    object->setUseDisplayList(true);

    // center object in scene
    object->setLocalPos(-1.0 * object->getBoundaryCenter());

	//tanslate & rotate
	object->translate(0.0, -0.5,0.2);


    // rotate object in scene
    object->rotateExtrinsicEulerAnglesDeg(0, 0, 180, C_EULER_ORDER_XYZ);

    // compute all edges of object for which adjacent triangles have more than 40 degree angle
    object->computeAllEdges(40);

    // set line width of edges and color
    cColorf colorEdges;
    colorEdges.setBlack();
    object->setEdgeProperties(1, colorEdges);

    // set normal properties for display
    cColorf colorNormals;
    colorNormals.setOrangeTomato();
    object->setNormalsProperties(0.01, colorNormals);

    // display options
    object->setShowTriangles(showTriangles);
    object->setShowEdges(showEdges);
    object->setShowNormals(showNormals);

	com_ball = new cShapeSphere(0.04);
	com_ball->setUseCulling(false);
	
	com_ball->setUseDisplayList(true);
	com_ball->m_material->setRed();
	com_ball->setUseVertexColors(true);
	

	
	world->addChild(com_ball);

	surface_line = new cShapeLine();
	surface_line->setUseCulling(false);

	surface_line->setUseDisplayList(true);
	surface_line->m_material->setRed();
	surface_line->setUseVertexColors(true);
	surface_line->m_colorPointA.setRed();
	surface_line->m_colorPointB.setRed();


	//world->addChild(surface_line);

    //--------------------------------------------------------------------------
    // WIDGETS
    //--------------------------------------------------------------------------

    // create a font
    font = NEW_CFONTCALIBRI20();
    
    // create a label to display the haptic and graphic rate of the simulation
    labelRates = new cLabel(font);
    labelRates->m_fontColor.setWhite();
    camera->m_frontLayer->addChild(labelRates);

    // create a background
    background = new cBackground();
    camera->m_backLayer->addChild(background);

    // set background properties
    //background->setCornerColors(cColorf(0.95f, 0.95f, 0.95f),
    //                            cColorf(0.95f, 0.95f, 0.95f),
    //                            cColorf(0.80f, 0.80f, 0.80f),
    //                            cColorf(0.80f, 0.80f, 0.80f));

	background->setCornerColors(cColorf(0.1f, 0.1f, 0.1f),
								cColorf(0.1f, 0.1f, 0.1f),
								cColorf(0.1f, 0.1f, 0.1f),
								cColorf(0.1f, 0.1f, 0.1f));


    //--------------------------------------------------------------------------
    // START SIMULATION
    //--------------------------------------------------------------------------

    // create a thread which starts the main haptics rendering loop
    hapticsThread = new cThread();
    hapticsThread->start(updateHaptics, CTHREAD_PRIORITY_HAPTICS);

    // setup callback when application exits
    atexit(close);


    //--------------------------------------------------------------------------
    // MAIN GRAPHIC LOOP
    //--------------------------------------------------------------------------

    // call window size callback at initialization
    windowSizeCallback(window, width, height);

    // main graphic loop
    while (!glfwWindowShouldClose(window))
    {
        // get width and height of window
        glfwGetWindowSize(window, &width, &height);

        // render graphics
        updateGraphics();

        // swap buffers
        glfwSwapBuffers(window);

        // process events
        glfwPollEvents();

        // signal frequency counter
        freqCounterGraphics.signal(1);
    }

    // close window
    glfwDestroyWindow(window);

    // terminate GLFW library
    glfwTerminate();

    // exit
    return 0;
}

//------------------------------------------------------------------------------

void windowSizeCallback(GLFWwindow* a_window, int a_width, int a_height)
{
    // update window size
    width  = a_width;
    height = a_height;
}

//------------------------------------------------------------------------------

void errorCallback(int a_error, const char* a_description)
{
    cout << "Error: " << a_description << endl;
}

//------------------------------------------------------------------------------

void keyCallback(GLFWwindow* a_window, int a_key, int a_scancode, int a_action, int a_mods)
{
    // filter calls that only include a key press
    if ((a_action != GLFW_PRESS) && (a_action != GLFW_REPEAT))
    {
        return;
    }

    // option - exit
    else if ((a_key == GLFW_KEY_ESCAPE) || (a_key == GLFW_KEY_Q))
    {
        glfwSetWindowShouldClose(a_window, GLFW_TRUE);
    }

    // option - show/hide texture
    else if (a_key == GLFW_KEY_1)
    {
        bool useTexture = object->getUseTexture();
        object->setUseTexture(!useTexture);
    }

    // option - enable/disable wire mode
    else if (a_key == GLFW_KEY_2)
    {
        bool useWireMode = object->getWireMode();
        object->setWireMode(!useWireMode, true);
    }

    // option - show/hide collision detection tree
    else if (a_key == GLFW_KEY_3)
    {
        cColorf color = cColorf(1.0, 0.0, 0.0);
        object->setCollisionDetectorProperties(collisionTreeDisplayLevel, color, true);
        bool show = object->getShowCollisionDetector();
        object->setShowCollisionDetector(!show, true);
    }

    // option - decrease depth level of collision tree
    else if (a_key == GLFW_KEY_4)
    {
        collisionTreeDisplayLevel--;
        if (collisionTreeDisplayLevel < 0) { collisionTreeDisplayLevel = 0; }
        cColorf color = cColorf(1.0, 0.0, 0.0);
        object->setCollisionDetectorProperties(collisionTreeDisplayLevel, color, true);
        object->setShowCollisionDetector(true, true);
    }

    // option - increase depth level of collision tree
    else if (a_key == GLFW_KEY_5)
    {
        collisionTreeDisplayLevel++;
        cColorf color = cColorf(1.0, 0.0, 0.0);
        object->setCollisionDetectorProperties(collisionTreeDisplayLevel, color, true);
        object->setShowCollisionDetector(true, true);
    }

    // option - save screenshot to file
    else if (a_key == GLFW_KEY_S)
    {
        cImagePtr image = cImage::create();
        camera->copyImageBuffer(image);
        image->saveToFile("screenshot.png");
        cout << "> Saved screenshot to file.       \r";
    }

    // option - show/hide triangles
    else if (a_key == GLFW_KEY_T)
    {
        showTriangles = !showTriangles;
        object->setShowTriangles(showTriangles);
    }

    // option - show/hide edges
    else if (a_key == GLFW_KEY_E)
    {
        showEdges = !showEdges;
        object->setShowEdges(showEdges);
    }

    // option - show/hide normals
    else if (a_key == GLFW_KEY_N)
    {
        showNormals = !showNormals;
        object->setShowNormals(showNormals);
    }

    // option - toggle fullscreen
    else if (a_key == GLFW_KEY_F)
    {
        // toggle state variable
        fullscreen = !fullscreen;

        // get handle to monitor
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();

        // get information about monitor
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        // set fullscreen or window mode
        if (fullscreen)
        {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
        else
        {
            int w = 0.8 * mode->height;
            int h = 0.5 * mode->height;
            int x = 0.5 * (mode->width - w);
            int y = 0.5 * (mode->height - h);
            glfwSetWindowMonitor(window, NULL, x, y, w, h, mode->refreshRate);
            glfwSwapInterval(swapInterval);
        }
    }

    // option - toggle vertical mirroring
    else if (a_key == GLFW_KEY_M)
    {
        mirroredDisplay = !mirroredDisplay;
        camera->setMirrorVertical(mirroredDisplay);
    }


	// option - toggle vertical mirroring
	else if (a_key == GLFW_KEY_M)
	{
		mirroredDisplay = !mirroredDisplay;
		camera->setMirrorVertical(mirroredDisplay);
	}

	// option - add water
	else if (a_key == GLFW_KEY_A)
	{
		w_bottle->set_water_flag(1);
	}

	// option - remove water
	else if (a_key == GLFW_KEY_B)
	{
		w_bottle->set_water_flag(2);
	}

	// option - stop water
	else if (a_key == GLFW_KEY_C)
	{
		w_bottle->set_water_flag(0);
	}
}

//------------------------------------------------------------------------------

void close(void)
{
    // stop the simulation
    simulationRunning = false;

    // wait for graphics and haptics loops to terminate
    while (!simulationFinished) { cSleepMs(100); }

    // close haptic device
    tool->stop();

    // delete resources
    delete hapticsThread;
    delete world;
    delete handler;
}

//------------------------------------------------------------------------------

void updateGraphics(void)
{
    /////////////////////////////////////////////////////////////////////
    // UPDATE WIDGETS
    /////////////////////////////////////////////////////////////////////

    // update haptic and graphic rate data
    labelRates->setText(cStr(freqCounterGraphics.getFrequency(), 0) + " Hz / " +
        cStr(freqCounterHaptics.getFrequency(), 0) + " Hz / "+cStr(w_bottle->calculate_com(),3) + " N ");

    // update position of label
    labelRates->setLocalPos((int)(0.5 * (width - labelRates->getWidth())), 15);


    /////////////////////////////////////////////////////////////////////
    // RENDER SCENE
    /////////////////////////////////////////////////////////////////////

    // update shadow maps (if any)
    world->updateShadowMaps(false, mirroredDisplay);

    // render world
    camera->renderView(width, height);

    // wait until all GL commands are completed
    glFinish();
	
	
    // check for any OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) cout << "Error: " << gluErrorString(err) << endl;
}

//------------------------------------------------------------------------------

enum cMode
{
    IDLE,
    SELECTION
};


double prev_scale = 1.0;
double applied_scale;
double testtest = 10;
std::mutex m;



double kk = 0;
double mm = 1;
void updateHaptics(void)
{
    cMode state = IDLE;
    cGenericObject* selectedObject = NULL;
    cTransform tool_T_object;

    // simulation in now running
    simulationRunning  = true;
    simulationFinished = false;
	double prev_time = glfwGetTime();
    // main haptic simulation loop
    while(simulationRunning)
    {

		//double current_time = glfwGetTime();
		//double elapsed_time = current_time - prev_time;
		//if (elapsed_time >= 1.0)
		//{


		//	prev_time = current_time;
		//}

			
		


		
        /////////////////////////////////////////////////////////////////////////
        // HAPTIC RENDERING
        /////////////////////////////////////////////////////////////////////////

        // signal frequency counter
        freqCounterHaptics.signal(1);

        // compute global reference frames for each object
        world->computeGlobalPositions(true);

        // update position and orientation of tool
        tool->updateFromDevice();

        // compute interaction forces
        tool->computeInteractionForces();

	

 
        /////////////////////////////////////////////////////////////////////////
        // MANIPULATION
        /////////////////////////////////////////////////////////////////////////

        // compute transformation from world to tool (haptic device)
        cTransform world_T_tool = tool->getDeviceGlobalTransform();

        // get status of user switch
        bool button = tool->getUserSwitch(0);

        //
        // STATE 1:
        // Idle mode - user presses the user switch
        //
        if ((state == IDLE) && (button == true))
        {
            // check if at least one contact has occurred
            if (tool->m_hapticPoint->getNumCollisionEvents() > 0)
            {
                // get contact event
                cCollisionEvent* collisionEvent = tool->m_hapticPoint->getCollisionEvent(0);

                // get object from contact event
                selectedObject = collisionEvent->m_object;
            }
            else
            {
                selectedObject = object;
            }

            // get transformation from object
            cTransform world_T_object = selectedObject->getGlobalTransform();

            // compute inverse transformation from contact point to object 
            cTransform tool_T_world = world_T_tool;
            tool_T_world.invert();

            // store current transformation tool
            tool_T_object = tool_T_world * world_T_object;

            // update state
            state = SELECTION;
        }


        //
        // STATE 2:
        // Selection mode - operator maintains user switch enabled and moves object
        //
        else if ((state == SELECTION) && (button == true))
        {
            // compute new transformation of object in global coordinates
            cTransform world_T_object = world_T_tool * tool_T_object;

            // compute new transformation of object in local coordinates
            cTransform parent_T_world = selectedObject->getParent()->getLocalTransform();
            parent_T_world.invert();
            cTransform parent_T_object = parent_T_world * world_T_object;

            // assign new local transformation to object
            selectedObject->setLocalTransform(parent_T_object);

            // set zero forces when manipulating objects
            tool->setDeviceGlobalForce(0.0, 0.0, 0.0);

            tool->initialize();
        }

        //
        // STATE 3:
        // Finalize Selection mode - operator releases user switch.
        //
        else
        {
            state = IDLE;
        }

		cVector3d force(0, 0, 0);
		cVector3d torque(0, 0, 0);
		double gripperForce = 0.0;

		// desired position
		cVector3d desiredPosition;
		desiredPosition.set(0.0, 0.0, 0.0);

		/////////////////////////////////////////////////////////////////////
		// APPLY FORCES


		cMatrix3d rot_mat;
		hapticDevice->getRotation(rot_mat);
		cVector3d cartesian = cVector3d(0.0, 0.0, 1.0);
		rot_mat.mul(cartesian);

		w_bottle->cartesian = cartesian;
		double r = cartesian.length();
		double theta = acos(cartesian.z() / r);
		double phi = atan(cartesian.y() / cartesian.x());
		w_bottle->phi = phi;

		//cout << "theta: " << theta << endl;

		w_bottle->set_tilted_angle(theta, phi);


		if (w_bottle->get_water_flag() == 1)
		{
			w_bottle->add_water();
		}
		else if (w_bottle->get_water_flag() == 2)
		{
			w_bottle->remove_water();
		}

		cVector3d gravity;
		w_bottle->calculate_volume();
		float water_weight = w_bottle->calculate_com();
		gravity.set(0.0, 0.0, -water_weight);
		force.add(gravity);
		cVector3d ocil = cVector3d(0.0, cos(kk), 0.0);
		//cVector3d ocil = cVector3d(0.0, mm, 0.0);
		/*force.add(ocil);*/
		mm = -mm;
		kk = kk + 0.1;
		
		cVector3d com_torque;
		com_torque.set(0.1, 0.0, 0.0);
		//torque.add(com_torque);
		cVector3d asdf = w_bottle->calculate_com_real(&cx, &cy);
		/*asdf = asdf / 5.0;*/
		torque.add(asdf);
		tool->setDeviceGlobalTorque(torque);
		double current_time = glfwGetTime();
		double elapsed_time = current_time - prev_time;
		cVector3d acc = tool->getDeviceGlobalLinVel();
		w_bottle->accel = acc;

		if (elapsed_time >= 1.0)
		{

			/*cout << "torque:  " << torque << endl;*/
			//cout << "local pos: " << cup->getLocalPos() << endl;
			//cout << "global pos: " << cup->getGlobalPos() << endl;
			cVector3d g_pos = tool->getDeviceGlobalPos();
			cVector3d ocil;
			
			cout << "height1: " << w_bottle->water_height1 << "  height2: " << w_bottle->water_height2 << endl;
			/*cout << phi << endl;*/
			//cout << "x: " << g_pos.x() << " y: " << g_pos.y() << " z: " << g_pos.z() << endl;
			//cout << "cx: " << cx << "  cy:  " << cy << endl;
			prev_time = current_time;
		}


		if (cartesian.y()> 0)
		{
			cx = -cx;
			force.add(cVector3d(0.0, cx*10.0*water_weight, 0.0));
		}
		cVector3d com_pos = cVector3d(0.0,cx,cy);
		//rot_mat.mul(com_pos);
		cVector3d result_com_pos;
		tool->setDeviceGlobalForce(force);
		tool->getDeviceGlobalTransform().mulr(com_pos,result_com_pos);
		
		//
		com_ball->setLocalPos(result_com_pos);
		com_ball->setRadius(0.04*(w_bottle->calculate_com()) / 5.0);


		cVector3d Apoint = cVector3d(0.0, 0.0, w_bottle->water_height1);
		cVector3d Bpoint = cVector3d(0.0, 2*w_bottle->radius, w_bottle->water_height2);
	
		tool->getDeviceGlobalTransform().mulr(Apoint, Apoint);
		tool->getDeviceGlobalTransform().mulr(Bpoint, Bpoint);

		surface_line->m_pointA = Apoint;
		surface_line->m_pointB = Bpoint;
		
		
		//cout << "test:  " << prev_scale << endl;
		//arrow->scale(1.0 / prev_scale);
		//testtest = testtest*(1 / prev_scale);
		//cout << "testoutput 1: " << testtest << endl;
		m.lock();
		arrow->scale(1.0 / prev_scale);
		if ((w_bottle->calculate_com()) / 5.0 == 0)
		{
			arrow->scale(0.001);
			prev_scale = 0.001;
		}
		else
		{
			arrow->scale((w_bottle->calculate_com()) / 5.0);
			prev_scale = (w_bottle->calculate_com()) / 5.0;
		}
			
	
		//testtest = testtest*0.7;
		//cout << "testoutput 2: " << testtest << endl;
		arrow->setLocalPos(result_com_pos);
		arrow->translate(cVector3d(0.0, 0.0, -0.1*((w_bottle->calculate_com()) / 5.0)));
		
		m.unlock();

		//arrow->scale((w_bottle->calculate_com()) / 5.0);

		//if ((w_bottle->calculate_com()) / 5.0 == 0)
		//{
		//	prev_scale = 1;
		//}
		//else
		//	prev_scale = (w_bottle->calculate_com()) / 5.0;
		
		
		//com_ball->translate(tool->getDeviceGlobalPos());
		//tool->setDeviceGlobalTorque(torque);
		tool->initialize();

        /////////////////////////////////////////////////////////////////////////
        // FINALIZE
        /////////////////////////////////////////////////////////////////////////

        // send forces to haptic device
        tool->applyToDevice();  
    }
    
    // exit haptics thread
    simulationFinished = true;
}

//------------------------------------------------------------------------------
