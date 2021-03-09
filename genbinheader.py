def create(filepath, varname, outfile, maxcolumn = 50):
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

# TODO: move binary header generatiion for each project here