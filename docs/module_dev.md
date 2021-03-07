# Module-Development
All modules are implemented in **client\modules**.

**Note: When adding or creating directories and C++ files for your module it's recommended to do it through Visual Studio (from the solution explorer)**.

1. Create a directory for your module to keep the modules folder clean.
    * eg. *"**client\modules\example_module**"*
2. Inside the directory you can implement your module's C++ files however you want, but it would be better to name them after your module.
    * eg.  **client\modules\example_module\example_module.h** and **client\modules\example_module\example_module.cpp**
3. Create your own implementation of the module base class (**patchii_module_base**) by including **client\patchii_module_base.h**.
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

4. Register the module in **client\modules\modules.cpp** by including your module's header then locate the function definition of patchii_get_registered_modules() then declare your module inside the vector array named **preload** using the **patchii_register_module(*module name*)** macro.
    * Example (This is an example, *client\modules\modules.cpp* may change overtime and some parts are removed to make this small):
        ```c++
        #include "modules.h"

        #include "example_module/example_module.h" // Include your module's header

        std::vector<patchii_module_base *> patchii_get_registered_modules()
        {
        	std::vector<std::pair<patchii_module_base *, const char *>> preload =
        	{
        		patchii_register_module(example_module) // Register it by adding it to the preload
        	};
        }
        ````

## Module-Interface

| Virtual function                 | Optional  | Load Required | Description                                                                                                                                                                                |
| ---------------------------------|:---------:|:-------------:| ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| bool load()                      | Depends   | No            | Called when patchii is loading the module. Return **true** if the module successfuly loaded and **false** otherwise.                                                                       |
| bool unload()                    | Depends   | Yes           | Called when patchii is unloading the module. Return **true** if the module successfuly unloaded and **false** otherwise.                                                                   |
| bool is_loaded()                 | No        | No            | Called when patchii is querying whether the module is loaded or not. Return **true** if the module is loaded and **false** if its unloaded.                                                |
| void draw_imgui_tools()          | Yes       | Yes           | Called when patchii is drawing the tools menu item in the main menu bar. Used for drawing custom ImGui controls in the **Tools** main menu item.                                           |
| void draw_imgui_mainmenubar()    | Yes       | Yes           | Called when patchii is drawing the main menu bar. Used for drawing custom ImGui controls in the main menu bar.                                                                             |
| void draw_imgui_module_options() | Yes       | No            | Called when patchii is drawing the options for the current module. Used for adding extra options aside from the **Load** and **Unload** in the **Modules** menu item in the main menu bar. |
| void draw_imgui()                | Yes       | Yes           | Called when patchii is drawing other ImGui controls. Used for general purpose drawing. (Windows, UI, etc...)                                                                               |
| void update()                    | Yes       | Yes           | Called when patchii is running its update cycle / main loop. Used for general purpose code execution.                                                                                      |
| void dxreset()                   | Yes       | Yes           | Called when DirectX device reset is called.                                                                                                                                                |