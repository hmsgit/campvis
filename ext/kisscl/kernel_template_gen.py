# Just a really simple script to automatically create the setArgument templates...

import string

def createTemplateDef(numArgs):
    toReturn = """/**
 * Set the """ + str(numArgs) + """ arguments of the kernel.
 * \\note    The number of arguments must match to the kernel.\n"""
    for i in range(0, numArgs):
        toReturn += ' * \\param   c' + str(i) + '   Argument ' + str(i+1) + '\n'
 
    toReturn += ' */\ntemplate<class C0'
    for i in range(1, numArgs):
        toReturn += ', class C' + str(i)
    toReturn += '>\n'
    
    toReturn += 'void setArguments(C0 co'
    for i in range(1, numArgs):
        toReturn += ', C' + str(i) + ' c' + str(i)
    toReturn += ');\n'

    return toReturn

def createTemplateDecl(numArgs):
    toReturn = 'template<class C0'
    for i in range(1, numArgs):
        toReturn += ', class C' + str(i)
    toReturn += '>\n'
    
    toReturn += 'void Kernel::setArguments(C0 c0'
    for i in range(1, numArgs):
        toReturn += ', C' + str(i) + ' c' + str(i)
    toReturn += ') {\n'

    toReturn += '\ttgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == ' + str(numArgs) + ', "Numer of arguments does not match the kernel\'s number of arguments.");\n\n'
    for i in range(0, numArgs):
        toReturn += '\tsetArgument<C' + str(i) + '>(' + str(i) + ', c' + str(i) + ');\n'

    toReturn += '}\n'
    return toReturn

def create(numArgs):
    toReturn = ''

    for i in range(1, numArgs+1):
        toReturn += createTemplateDef(i) + '\n'

    toReturn += '\n\n'

    for i in range(1, numArgs+1):
        toReturn += createTemplateDecl(i) + '\n'

    return toReturn

print(create(10))
