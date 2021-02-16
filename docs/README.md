# patchii2

<img src="ss.png" />

<table>
    <tr>
        <td><img src="patchii_img_128x201.png" height=50%/></td>
        <td><b>patchii2</b><br>module based anemic internet cafe poking tool</td>
    </tr>
</table>

[Features](#Features) • [Usage](#Usage) • [Development](#Development) • [Module Development](#Module-Development) • [License](#License) • [Libraries](#Libraries) • [Credits](#Credits)

Contributions through PR's are always welcome, check the [Issues](https://github.com/rogueeeee/patchii2/issues) for thing that you might be able to help with.

## Features

## Usage
Load the compiled DLL located in **build/*configuration*_*platform*/patchii_client.dll** followed by the appropriate build mode and target architecture to your target process.

## Development
[Repository Branches](#Repository-Branches) • [Prerequisite](#Prerequisite) • [Project Structure](#Project-Structure) • [Setup](#Setup) • [Building](#Building)

### Repository-Branches
n/a

### Prerequisite
* Windows
* [DirectX SDK](https://www.microsoft.com/en-ph/download/details.aspx?id=10568)
* [Visual Studio 2019](https://visualstudio.microsoft.com/)

### Project-Structure

| Directory    | Description                                                                                                                    |
| ------------ | ------------------------------------------------------------------------------------------------------------------------------ |
| client/      | Contains the main client that gets loaded to a target process, mainly contains all the features, implementations, and modules. |
| docs/        | Contains the project documentation.                                                                                            |
| impl_gui/    | Contains GUI Implementation of Win32 GUI, DirectX, and IMGui. Allows quick creation of GUI applications.                       |
| loader/      | Contains the loader project source code that loads the client into a target process.                                           |
| third_party/ | Contains 3rd-Party code and libraries.                                                                                         |
| utils/       | Contains general purpose utility code/libraries.                                                                               |

### Setup
* Open **patchii2.sln** in Visual Studio.
* It is recommended to use the **Show All Files** view mode for the Solution Explorer.
<img src="vs_showfiles.png" />

### Building
Open **patchii2.sln** in Visual Studio, configure it to your target build, and then start the build.

## Module-Development
All modules are implemented in **client/modules**.

**Note: When adding or creating directories and C++ files for your module it's recommended to do it through Visual Studio (from the solution explorer)**.

1. Create a directory for your module to keep the modules folder clean.
    * eg. *"**client/modules/example_module**"*
2. Inside the directory you can implement your module's C++ files however you want, but it would be better to name them after your module.
    * eg.  **client/modules/example_module/example_module.h** and **client/modules/example_module/example_module.cpp**
3. Create your own implementation of the module base class (**patchii_module_base**) by including **client/patchii_module_base.h**.
    * Make sure to use the **override** keyword when you're implementing one of the functions.
    * Read [Module Interface](#Module-Interface) for extensive documentation on what each virtual function is used for.
    * Note: Some functions are optional to override.
    * **example_module.h**
        ```c++
        #include <client/patchii_module_base.h>

        class example_module : public patchii_module_base
        {
        public:
        	example_module();

        public:
        	virtual bool load() override;
        	virtual bool unload() override;
        	virtual bool is_loaded() override;
        };
        ```

    * **example_module.cpp** - Use the initializer list of your module's constructor to call the *patchii_module_base*'s constructor to set the name of your module.
        ```c++
        #include "example_module.h"

        example_module::example_module()
            : patchii_module_base("example") // Your module's name
        {
        }
        ```

4. Register the module in **client/modules/modules.cpp** by including your module's header then locate the function definition of patchii_get_registered_modules() then declare your module inside the vector array named **preload** using the **patchii_register_module(*module name*)** macro.
    * Example (This is an example, *client/modules/modules.cpp* may change overtime and some parts are removed to make this small):
        ```c++
        #include "modules.h"

        #include "example_module/example_module.h" // Include you module's header

        std::vector<patchii_module_base *> patchii_get_registered_modules()
        {
        	std::vector<std::pair<patchii_module_base *, const char *>> preload =
        	{
        		patchii_register_module(example_module) // Register it by adding it to the preload
        	};
        }
        ````

### Module-Interface

| Virtual function                 | Optional  | Load Required | Description                                                                                                                                                                                |
| ---------------------------------|:---------:|:-------------:| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| bool load()                      | No        | No            | Called when patchii is loading the module. Return **true** if the module successfuly loaded and **false** otherwise.                                                                       |
| bool unload()                    | No        | Yes           | Called when patchii is unloading the module. Return **true** if the module successfuly unloaded and **false** otherwise.                                                                   |
| bool is_loaded()                 | No        | No            | Called when patchii is querying whether the module is loaded or not. Return **true** if the module is loaded and **false** if its unloaded.                                                |
| void draw_imgui_tools()          | Yes       | Yes           | Called when patchii is drawing the tools menu item in the main menu bar. Used for drawing custom ImGui controls in the **Tools** main menu item.                                           |
| void draw_imgui_mainmenubar()    | Yes       | Yes           | Called when patchii is drawing the main menu bar. Used for drawing custom ImGui controls in the main menu bar.                                                                             |
| void draw_imgui_module_options() | Yes       | No            | Called when patchii is drawing the options for the current module. Used for adding extra options aside from the **Load** and **Unload** in the **Modules** menu item in the main menu bar. |
| void draw_imgui()                | Yes       | Yes           | Called when patchii is drawing other ImGui controls. Used for general purpose drawing. (Windows, UI, etc...)                                                                               |
| void update()                    | Yes       | Yes           | Called when patchii is running its update cycle / main loop. Used for general purpose code execution.                                                                                      |
| void dxreset()                   | Yes       | Yes           | Called when DirectX device reset is called.                                                                                                                                                |

## License
[GNU General Public License 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html)

## Libraries
* [Dear IMGui](https://github.com/ocornut/imgui) - ocornut
* [MinHook](https://github.com/TsudaKageyu/minhook) - TsudaKageyu

## Credits
* [Patchouli Image](https://www.deviantart.com/fantastiic/art/Chibi-Patchouli-Knownledge-Touhou-305044472) - fantastiic 