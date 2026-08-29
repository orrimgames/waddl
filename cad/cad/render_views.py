import numpy as np, trimesh, copy
from trimesh.transformations import rotation_matrix as R, translation_matrix as T
import os
out = os.path.expanduser("~/waddl")
base = trimesh.load(out+"/assets/pip.glb")
c = base.bounds.mean(axis=0)
views = {
 "front":  R(0,(0,1,0)),
 "threequarter": R(np.radians(35),(0,1,0)) @ R(np.radians(-12),(1,0,0)),
 "side":   R(np.radians(90),(0,1,0)),
 "back":   R(np.radians(180),(0,1,0)),
 "top":    R(np.radians(-90),(1,0,0)),
}
for name, rot in views.items():
    s = trimesh.Scene()
    for gname, g in base.geometry.items():
        m = g.copy()
        m.apply_transform(T(-c))
        m.apply_transform(rot)
        m.apply_transform(T((0,0,0)))
        s.add_geometry(m, node_name=gname, geom_name=gname)
    img = s.save_image(resolution=(900,900), visible=False)
    open(f"{out}/assets/renders/pip_{name}.png","wb").write(img)
    print(name, len(img))
