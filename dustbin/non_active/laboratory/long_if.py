import random
alphabet = "ABCDEFGHIJKLMNOPQRSTUVXYZ"
actions = ['press','release']
for _ in range(int(1e4)):
    action = random.choice(actions)
    key = random.choice(alphabet)
    print("    if (key == GLFW_KEY_{} && {}(action)) {{printf(\"pressed {}.\\n\");}}".format(key, action, key))
