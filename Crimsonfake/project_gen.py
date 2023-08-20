from pathlib import Path
root_dir = Path.cwd()

import sys
sys.path.append(root_dir)
sys.path.append("../frameworks/RisingCore/tools/cppParser/src")
sys.path.append("../frameworks/RisingCore/tools/engineCodegen/src")
sys.path.append("../frameworks/RisingCore/tools/thirdparty/code_generator/src")

import project

if __name__ == "__main__":
    project.generate("workspace/project.json")
    