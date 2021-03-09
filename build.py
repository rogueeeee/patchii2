import os
import sys
import shutil
import codecs
import genbinheader
from distutils.spawn import find_executable

upx_enabled = True
build_depth = 2

def run_msbuild(project_name, platform):
    if int(os.system("msbuild patchii2.sln /t:" + project_name + " /p:TreatWarningsAsErrors=true /p:Configuration=Release /p:Platform=" + platform)) is not 0:
        print("\nBUILD FAILED\n")
        exit(1)
    return

def run_upx(path):
    if int(os.system("upx " + os.path.realpath(path) + " -9 -v -f")) is not 0:
        print("\nCOMPRESSING FAILED\n")
        exit(1)
    return

# Check parameters
for arg in sys.argv[1:]:
    if arg == "-noupx":
        upx_enabled = False
    elif arg == "-upx":
        upx_enabled = True
    elif arg == "-bd.client":
        build_depth = 0
    elif arg == "-bd.injector":
        build_depth = 1
    elif arg == "-bd.loader" or arg == "-bd.full":
        build_depth = 2
    else:
        print("Unknown parameter: " + arg)
        exit(1)

# Check for msbuild
if find_executable("msbuild") is None:
    print("ERROR: msbuild not found! Add the msbuild directory to your environment path variable")
    exit(1)

# Check for UPX
if upx_enabled and find_executable("upx") is None:
    print("ERROR: upx was not found! Add the upx directory to your environment path variable or use the -noupx parameter when executing the script")
    exit(1)

# Run cleanup
print("Running cleanup...")
if os.system("msbuild patchii2.sln /t:Clean") is not 0:
    print("Cleanup failed")
    exit(1)

# Build client
if 0 <= build_depth:
    print("Starting build of client/ x86")
    run_msbuild("client", "x86")
    print("Starting build of client/ x64")
    run_msbuild("client", "x64")

if 1 <= build_depth:
    # Generate client binary header for injector
    print("Generating binary headers for injector/")
    print("\tGenerating for x86...")
    genbinheader.create("build/Release_Win32/patchii_client.dll", "client_bin", "injector/binaries/bin_x86.h")
    print("\tGenerating for x64...")
    genbinheader.create("build/Release_x64/patchii_client.dll", "client_bin", "injector/binaries/bin_x64.h")
    # Build injector
    print("Starting build of injector/ x86")
    run_msbuild("injector", "x86")
    print("Starting build of injector/ x64")
    run_msbuild("injector", "x64")

if 2 <= build_depth:
    # Generate injector binary header for loader
    print("Generating binary headers for loader/")
    print("\tGenerating for x86...")
    genbinheader.create("build/Release_Win32/patchii_injector.exe", "inj_bin_x86", "loader/binaries/inj_binary_x86.h")
    print("\tGenerating for x64...")
    genbinheader.create("build/Release_x64/patchii_injector.exe", "inj_bin_x64", "loader/binaries/inj_binary_x64.h")
    # Build loader
    print("Starting build of injector/ x86")
    run_msbuild("loader", "x86")
    print("Starting build of injector/ x64")
    run_msbuild("loader", "x64")

print("\nBUILD FINISHED")

if build_depth == 2:
    os.makedirs("build/final", 0o777, True)
    shutil.copyfile("build/Release_Win32/patchii_loader.exe", "build/final/patchii_x86.exe")
    shutil.copyfile("build/Release_x64/patchii_loader.exe", "build/final/patchii_x64.exe")
    # Pack binaries
    if upx_enabled and build_depth == 2:
        print("\nPACKING\n")
        if os.system("upx build/final/patchii_x86.exe build/final/patchii_x64.exe -9 -v") is not 0:
            print("FAILED TO PACK BINARIES")
            exit(1)
    os.startfile(os.path.realpath("build/final"))

exit(0)