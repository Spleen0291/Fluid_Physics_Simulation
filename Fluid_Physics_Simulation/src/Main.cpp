
#include "../HeaderFiles/Shaders.h"
#include "../HeaderFiles/Particle.h"
#include "../HeaderFiles/Window.h"
#include <cmath>
#include <limits> // MAX_INT

// Defining static variables 
std::vector <float> Window::recData = {
    -0.9f,  0.9f,
     0.9f,  0.9f,
     0.9f, -0.9f,
    -0.9f, -0.9f
};

std::vector <float> Particle::centers = {};

int Particle::segments = 16;
float Particle::spacing = 0.005f;
float Particle::stepSize = 0.0005f;
int Particle::numOfParticles = 2000;
float Particle::radius = 0.008f;
float Particle::s_Radius = 0.05f;
float Particle::targetDensity = 400.0f;
float Particle::pressureMultiplier = 200.0f;
float Particle::nearPressureMultiplier = 1000.0f;
float Particle::viscosityMultiplier = 0.0002f;

int main(int numArgs, const char *aArgs[])
{
    Window window(1600, 1000);
    
    // Generating Buffers
    glGenVertexArrays(1, &Window::vao);
    glGenBuffers(1, &Window::vbo);

    glGenVertexArrays(1, &Particle::vao);
    glGenBuffers(1, &Particle::vbo);
    glGenBuffers(1, &Particle::ibo);

    Particle::generateGridCenters(20, 25); // generate grid / random particles
    Particle::populate(window.aspectRatio); // create particles using center positions

    // creating and compiling shaders
    Shader::shaderProgramSource source = Shader::parse("res/shaders/Basic.shader");
    unsigned int shader = Shader::create(source.vertexSource, source.fragmentSource);
    glUseProgram(shader);

    // Uniforms Declaration
    int color_Location = glGetUniformLocation(shader, "u_Color");
    glUniform3f(color_Location, 0.2f, 0.3f, 0.8f);

    int object_Location = glGetUniformLocation(shader, "u_pos");
    glUniform4f(object_Location, 0.0f, 0.0f, 0.0f, 0.0f);

    /* Loop until the user closes the window */

    bool do_benchmark                   = numLastPhysicsSeconds > 0.0;
    static double lastTime              = 0.0f;
    static double elapsed               = 0.0;
    static int    numFrame              = 0;
    static int    numFirstRenderFrame   = 2*60 * 60; // 2 min -> 2*60 -> 120 seconds * 60 fps/s = 7,200 frames
    static double numLastPhysicsSeconds = 3.0 * 60.0; // 3 minutes * 60secs/min = 180 seconds

    if (numArgs > 1)
    {
        if (numArgs > 1) {
            numFirstRenderFrame = atoi( aArgs[1] );
            if (numFirstRenderFrame < 0)
                numFirstRenderFrame = INT_MAX;
        }
        if (numArgs > 2) {
            numLastPhysicsSeconds = atof( aArgs[2] );
            if (numLastPhysicsSeconds < 0.0)
                numLastPhysicsSeconds = 0.0;
        }
    }
#if USE_CPP_IOSTREAM
    std::cout.precision(6);
    std::cout
        << "Configuration: (C++ iostream)" << std::endl
        << std::fixed
        << "    First Render Frame: # " <<                                         numFirstRenderFrame   << std::endl
        << "    Last Physics Seconds: " << std::setw(7) << std::setprecision(3) << numLastPhysicsSeconds << std::endl;
#else
    printf( "Configuration: (C printf)\n" );
    printf( "    First Render Frame: # %d\n", numFirstRenderFrame );
    printf( "    Last Physics Seconds: %7.3f\n", numLastPhysicsSeconds );
#endif

    while (!glfwWindowShouldClose(window.win))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        Window::drawBoundary(object_Location, color_Location);
        bool bDraw = (numFrame >= numFirstRenderFrame);
        Particle::drawElements(window, object_Location, color_Location, bDraw);

        //calculate fps
        numFrame++;
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
               elapsed += deltaTime; // POHO
        lastTime = currentTime;
#if USE_CPP_IOSTREAM
        std::cout
            << "FPS: "          << std::setw(7) << std::setprecision(3) << (1.f / deltaTime)
            << " / Frametime: " << std::setw(7) << std::setprecision(3) << deltaTime * 1000.f << "ms"
            << "  Frame #: "    << std::setw(7)                         << numFrame
            << "  Elapsed: "    << std::setw(7) << std::setprecision(3) << elapsed << " s"
            << std::endl;
#else
        printf( "FPS: %7.3f / Frametime: %7.3f ms  Frame #: %7d  Elapsed: %7.3f s\n", (1.f / deltaTime), deltaTime * 1000.f, numFrame, elapsed );
#endif
        /* Swap front and back buffers */
        glfwSwapBuffers(window.win);

        /* Poll for and process events */
        glfwPollEvents();
        if (do_benchmark && (elapsed >= numLastPhysicsSeconds)) break;
    }

    double frames  = (double)numFrame; // frames
    double avgFPS  = frames / elapsed; // frames/second
    double avgFTms = (1.0 / avgFPS) * 1000.0; // ms
#if USE_CPP_IOSTREAM
    std::cout
        <<   "Total Frames: "  <<                                         numFrame << " "
        << "/ Total Elapsed: " << std::setw(7) << std::setprecision(3) << elapsed << " s "
        << "= Avg FPS: "       << std::setw(7) << std::setprecision(3) << avgFPS
        << ", Avg Frametime: " << std::setw(7) << std::setprecision(3) << avgFTms << " ms"
        << std::endl;
#else
    printf( "Total Frames: %d / Total Elapsed: %7.3f s = Avg FPS: %7.3f, Avg Frametime: %7.3f ms \n", numFrame, elapsed , avgFPS, avgFTms );
#endif

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}




/*  TODO:
    1.Particle and Bounding box interactions
        --a) Create a bounding box--
        --b) Make visible boundries--
        c) Test using different and multiple particle positions and speeds

    2.Particle - Particle interactions
    3.Fluid mechanics:
        Density
        Viscosity
        Make use of SPH model
    4.Testing fluid behaviour to different objects and containers
    5.Fluid interactions with user inputs
    6.Add color gradients to make fluid presentable
    7.Final touches and bug testing and fixing
    8.Upload to github and plan for future developments...
*/