from operator import truediv

from behavior_tree.Node import Node
class Selector(Node):
    """
    追逐玩家
    :param t_x : 目标x
    :param t_y : 目标y
    """
    def __init__(self):
        self.node_list = []

    def append_node(self,node: Node):
        self.node_list.append(node)
    def execute(self,blackboard):
        for node in self.node_list:
            if node.execute(blackboard):
                return True
        return False

