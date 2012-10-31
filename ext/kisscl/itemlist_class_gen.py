# Just a really simple script to automatically create the setArgument templates...

import string

def argList(numArgs):
    toReturn = 'const T& i1'
    for i in range(2, numArgs+1):
        toReturn += ', const T& i' + str(i)
    return toReturn;

def createTemplateDef(numArgs):
    toReturn = """/**
 * Wraps around the ids of a fixed list of OpenCL items.
 */
template <typename T>
struct ItemList {
    typedef typename T::cl_type ItemType;   ///< Typedef for the OpenCL id type of the item.

    /// Creates an empty ItemList
    explicit ItemList();

"""

    for j in range(1, numArgs+1):
        toReturn += '    /**\n     * Creates an ItemList containing ' + str(j) + ' items.\n'
        for i in range(1, j+1):
            toReturn += '     * \\param   i' + str(i) + '   Item ' + str(i) + '\n'
        toReturn += '     */\n'

        toReturn += '    explicit ItemList(' + argList(j) + ');\n\n'

    toReturn += """    /// Destructor
    ~ItemList();

    size_t _size;           ///< Number of items in this list.
    ItemType* _items;       ///< Pointer to the Ids if the items.
};"""
    
    return toReturn

def createTemplateDecl(numArgs):
    toReturn = """template <typename T>
kisscl::ItemList<T>::ItemList()
    : _size(0)
    , _items(0)
{
}
"""

    for i in range(1, numArgs+1):
        toReturn += """template <typename T>
kisscl::ItemList<T>::ItemList(""" + argList(i) + """)
    : _size(""" + str(i) + """)
    , _items(0)
{
    _items = new ItemType[_size];
"""
        for j in range(0, i):
            toReturn += '    _items[' + str(j) + '] = i' + str(j+1) + '.getId();\n'
        toReturn += '}\n\n'

    toReturn += """
template <typename T>
kisscl::ItemList<T>::~ItemList() {
    delete _items;
}"""

    return toReturn

def create(numArgs):
    toReturn = createTemplateDef(numArgs)
    toReturn += '\n\n'
    toReturn += createTemplateDecl(numArgs)

    return toReturn

print(create(10))
