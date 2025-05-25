from Ai_level.Ai_level import AiLevel
from behavior_tree.SelectNode import Selector
from behavior_tree.Chase import Chase


class AiLevel1(AiLevel):
    def __init__(self):
        self.selector = Selector()
        self.selector.append_node(Chase())

    def update(self,blackboard):
        self.selector.execute(blackboard)


