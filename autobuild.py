import os
import sys
import shutil
import codecs

from distutils.spawn import find_executable

x86 = 0
x64 = 1

def run_msbuild(project_name, platform):
    if int(os.system("msbuild patchii2.sln /t:" + project_name + " /p:Configuration=Release /p:Platform=x" + ("86" if platform == x86 else "64"))) is not 0:
        print("\nBUILD FAILED\n")
        exit(1)
    return

def generate_binary_header(filepath, varname, outfile, maxcolumn = 50):
    binarybytes = open(filepath, 'rb').read()
    cheadercontent = "#pragma once\n\nunsigned char " + varname + "[] =\n{\n\t"
    currentcolumn = 0
    for idx in range(0, len(binarybytes)):
        currentcolumn += 1
        if currentcolumn == maxcolumn:
            cheadercontent += "\n\t"
            currentcolumn = 0
        cheadercontent += hex(binarybytes[idx]) + ", "
    cheadercontent += "\n};"
    open(outfile, "w").write(cheadercontent)
    return

print("")
print("patchii auto build script")
print("  * It's recommended to build from the stable branch")
print("  * Make sure the directory where msbuild is installed at is in your environment variable path")
print("")

if find_executable("msbuild") is None:
    print("ERROR: msbuild not found! Add the msbuild directory to your environment path variable")
    exit(1)

print("Running cleanup...")
if os.system("msbuild patchii2.sln /t:Clean") is not 0:
    print("Cleanup failed")
    exit(1)

print("Starting build of client/ x86")
run_msbuild("client", x86)

print("Starting build of client/ x64")
run_msbuild("client", x64)

print("Generating binary headers for injector/")
print("\tGenerating for x86...")
generate_binary_header("build/Release_Win32/patchii_client.dll", "client_bin", "injector/binaries/bin_x86.h")
print("\tGenerating for x64...")
generate_binary_header("build/Release_x64/patchii_client.dll", "client_bin", "injector/binaries/bin_x64.h")

print("Starting build of injector/ x86")
run_msbuild("injector", x86)

print("Starting build of injector/ x64")
run_msbuild("injector", x64)

print("Generating binary headers for loader/")
print("\tGenerating for x86...")
generate_binary_header("build/Release_Win32/patchii_injector.exe", "inj_bin_x86", "loader/binaries/inj_binary_x86.h")
print("\tGenerating for x64...")
generate_binary_header("build/Release_x64/patchii_injector.exe", "inj_bin_x64", "loader/binaries/inj_binary_x64.h")

print("Starting build of injector/ x86")
run_msbuild("loader", x86)

print("Starting build of injector/ x64")
run_msbuild("loader", x64)

print("\nBUILD FINISHED")
os.makedirs("build/final", 0o777, True)
shutil.copyfile("build/Release_Win32/patchii_loader.exe", "build/final/patchii_x86.exe")
shutil.copyfile("build/Release_x64/patchii_loader.exe", "build/final/patchii_x64.exe")
os.startfile(os.path.realpath("build/final"))

exit(0)