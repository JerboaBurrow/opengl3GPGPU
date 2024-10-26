#include <glGPGPU.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>

const char * computeShader =
    "#version " GLSL_VERSION "\n"
    "precision highp float;\n"
    "precision highp int;\n"
    "in vec2 o_texCoords;\n"
    "layout(location=0) out float output;\n"
    "uniform highp sampler2D x;\n"
    "uniform highp sampler2D y;\n"
    "void main(){\n"
    "    output = texture(x, o_texCoords).r+texture(y, o_texCoords).r;\n"
    "}";

const int n = 16;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow * glfwWindow = glfwCreateWindow(1,1,"glGPGPU",NULL,NULL);
    glfwSwapInterval(1);
    glfwMakeContextCurrent(glfwWindow);
    glewInit();

    glCompute compute
    (
        {
            {"x", {n, n}},
            {"y", {n, n}}
        },
        {n, n},
        computeShader
    );

    std::vector<float> x(n*n, 0.0);
    std::vector<float> y(n*n, 0.0);

    for (int i = 0; i < n*n; i++)
    {
        x[i] = 2*i;
        y[i] = -i;
    }

    compute.set("x", x);
    compute.set("y", y);
    compute.sync();
    compute.compute(true);
    auto output = compute.result();
    for (int i = 0; i < n*n-1; i++)
    {
        std::cout << output[i] << ", ";
    }
    std::cout << output[n*n-1] << "\n";

    glfwSwapBuffers(glfwWindow);
    glfwWindowShouldClose(glfwWindow);
}