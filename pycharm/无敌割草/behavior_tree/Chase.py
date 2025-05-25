from behavior_tree.Node import Node
import math
class Chase(Node):
    """
    追逐玩家
    :param t_x : 目标x
    :param t_y : 目标y
    """
    def execute(self,blackboard: dict):
        #   判断是否存在目标位置
        if 't_x' not in blackboard or blackboard["t_x"] is None:
            return False
        #   判断是否存在位置坐标
        if "x" in blackboard and "y" in blackboard and "move_speed" in blackboard:
            dx = blackboard["t_x"] - blackboard["x"]
            dy = blackboard["t_y"] - blackboard["y"]
            distance = math.hypot(dx ,dy)
            if distance > 0:
                v = blackboard["move_speed"]
                blackboard["x"] += (dx / distance) * v
                blackboard["y"] += (dy / distance) * v
            return True
        return False

