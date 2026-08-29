"""Pip v0.1 - cute duck companion robot. Builds the GLB for the web sim.
Y-up, 1 unit = 1cm. All parts named for animation."""
import numpy as np, trimesh
from trimesh.transformations import translation_matrix as T, rotation_matrix as R
def S(v): return np.diag([v[0],v[1],v[2],1.0])

CREAM=(245,238,220,255); ORANGE=(240,145,45,255); DARK=(24,28,38,255)
CYAN=(90,225,255,255)

def sphere(r, scale=(1,1,1), color=CREAM):
    m = trimesh.creation.icosphere(subdivisions=3, radius=r)
    m.apply_transform(np.diag([scale[0],scale[1],scale[2],1.0]))
    m.visual.face_colors = color
    return m

def _sphere_old(r, scale=(1,1,1), color=CREAM):
    m = trimesh.creation.icosphere(subdivisions=3, radius=r)
    m.apply_transform(S((*scale,)))
    m.visual.face_colors = color
    return m

scene = trimesh.Scene()
def add(mesh, name, pos=(0,0,0), rot=None):
    if rot is not None: mesh.apply_transform(rot)
    mesh.apply_transform(T(pos))
    scene.add_geometry(mesh, node_name=name, geom_name=name)

add(sphere(7.0,(1.0,0.92,1.08)), "body", (0,7.2,0))
add(sphere(5.6,(0.82,0.78,0.9),color=(252,248,236,255)), "belly", (0,6.4,3.4))
add(sphere(5.2,(1.05,0.95,1.0)), "head", (0,14.2,1.2))
add(sphere(3.9,(1.0,0.82,0.40),color=DARK), "face", (0,15.0,6.1))
for sx, nm in ((-1,"eye_l"),(1,"eye_r")):
    add(sphere(0.95,(0.8,1.15,0.4),color=CYAN), nm, (sx*1.75,15.4,8.15))
beak = trimesh.creation.cone(radius=1.9, height=2.6, sections=24)
beak.apply_transform(S((1.25,0.62,1.0)))
beak.apply_transform(R(np.pi/2,(1,0,0)))
beak.visual.face_colors = ORANGE
add(beak, "beak", (0,12.5,7.0))
for sx, nm in ((-1,"wing_l"),(1,"wing_r")):
    w = sphere(2.6,(0.55,1.0,1.35),color=(238,228,205,255))
    add(w, nm, (sx*7.0,7.6,0.2), R(sx*-0.28,(0,0,1)))
for sx, nm in ((-1,"foot_l"),(1,"foot_r")):
    foot = sphere(2.3,(1.15,0.42,1.5),color=ORANGE)
    add(foot, nm, (sx*3.1,0.9,0.9))
    for i,dx in enumerate((-1.2,0,1.2)):
        add(sphere(0.85,(1.0,0.45,1.4),color=ORANGE), f"{nm}_toe{i}", (sx*3.1+dx,0.85,3.1))
tail = trimesh.creation.cone(radius=1.6, height=3.0, sections=20)
tail.apply_transform(S((1.2,1.0,0.8)))
tail.apply_transform(R(-0.7,(1,0,0)))
tail.visual.face_colors = (238,228,205,255)
add(tail, "tail", (0,11.5,-6.6))

import os
out = os.path.expanduser("~/waddl")
scene.export(out+"/assets/pip.glb")
scene.geometry["body"].export(out+"/cad/pip_body_shell.stl")
print("bounds:", np.round(scene.bounds,2).tolist())
print("nodes:", sorted(scene.geometry.keys()))
