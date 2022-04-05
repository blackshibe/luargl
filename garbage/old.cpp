// #define GLFW_INCLUDE_VULKAN
// #include <GLFW/glfw3.h>

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>

// #include <iostream>

// int main() {
//     glfwInit();

//     glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
//     GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

//     uint32_t extensionCount = 0;
//     vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

//     std::cout << extensionCount << " extensions supported\n";

//     glm::mat4 matrix;
//     glm::vec4 vec;
//     auto test = matrix * vec;

//     while(!glfwWindowShouldClose(window)) {
//         glfwPollEvents();
//     }

//     glfwDestroyWindow(window);
//     glfwTerminate();

//     return 0;
// }
// C++ program to illustrate the use of
// LuaCpp library

// stfu
#include <LuaCpp.hpp>
#include <iostream>
#include <fstream>
#include <string>

using namespace LuaCpp;
using namespace LuaCpp::Registry;
using namespace LuaCpp::Engine;
using namespace std;

// useful shit:
// reading from and to files: https://www.tutorialspoint.com/read-data-from-a-text-file-using-cplusplus
  
extern "C" {
    static int _sum (lua_State *L) {
        int n = lua_gettop(L);    /* number of arguments */
        lua_Number sum = 0.0;
        
        int i;
        for (i = 1; i <= n; i++) {
            if (!lua_isnumber(L, i)) {
                lua_pushliteral(L, "incorrect argument");
                lua_error(L);
            }
            sum += lua_tonumber(L, i);
        }

        lua_pushnumber(L, sum);         /* second result */
        return 1;                   /* number of results */
    }
}

// Driver Code
int main(int argc, char** argv)
{
  
    // fstream newfile;
    // LuaContext lua;

    // newfile.open("./project/script/main.lua",ios::out);  // open a file to perform write operation using file object
    // if(newfile.is_open())     //checking whether the file is open
    // {
    //     newfile<<"Tutorials point \n"; //inserting text
    //     newfile.close(); //close the file object
    // }

    	// Create library "foo" conatining the "foo" function
	// std::shared_ptr<LuaLibrary> lib = std::make_shared<LuaLibrary>("foolib");

	// lib->AddCFunction("sum", _sum);
	// // Add library to the context
	// lua.AddLibrary(lib);


    // newfile.open("./project/script/main.lua",ios::in); //open a file to perform read operation using file object
    // if (newfile.is_open()){   //checking whether the file is open
    //     string temporary;
    //     string file_contents;

    //     while(getline(newfile, temporary)){
    //         file_contents = file_contents + temporary + "\n";
    //     }
    //     newfile.close(); 

    //     std::shared_ptr<Engine::LuaTBoolean> str = std::make_shared<Engine::LuaTBoolean>(false);
    //     lua.AddGlobalVariable("funny", str);

    //     try {
    //         cout << "running lua file\n";
    //         lua.CompileStringAndRun(file_contents);
    //     } catch (std::runtime_error& e) {
    //         std::cout << e.what()
    //                 << '\n';
    //     }


    //     cout << str->getValue() << "\n";

    // }
}