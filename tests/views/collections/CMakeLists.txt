# ListView 仅为视图库组件；FluentListItemDelegate 与 model 组装放在测试中作为业务示例。
add_qt_test_module(test_list_view TestListView.cpp FluentListItemDelegate.cpp)

# GridView
add_qt_test_module(test_grid_view TestGridView.cpp FluentGridItemDelegate.cpp)

# TreeView
add_qt_test_module(test_tree_view TestTreeView.cpp FluentTreeItemDelegate.cpp)

# FlipView
add_qt_test_module(test_flip_view TestFlipView.cpp)
