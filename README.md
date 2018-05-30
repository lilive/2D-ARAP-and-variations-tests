I'm looking for a good way to deform a plane 2D mesh by moving only a subset of its vertices.

This is a short study about differents availables methods, for use in openFrameworks applications.

There is the [ofxPuppet](https://github.com/ofZach/ofxPuppet) addon for openFrameworks. This is great because it provide a easy way to do As-Rigid-As-Possible (ARAP) deformation, but I'm not so happy with the results.

So I made some tries with the [CGAL library](https://www.cgal.org/index.html). CGAL provides [3 differents methods](https://doc.cgal.org/latest/Surface_mesh_deformation/index.html#Chapter_SurfaceMeshDeformation) :

- The As-Rigid-As-Possible (ARAP) method
- The Spokes and Rims method
- The Smoothed Rotation Enhanced As-Rigid-As-Possible method

This application show the results given by ofxPuppet and the 3 CGAL methods :

![mesh-deformation-comparison.gif](mesh-deformation-comparison.gif)

### Require

- [openFrameworks](https://openframeworks.cc/)
- [ofxPuppet](https://github.com/ofZach/ofxPuppet)
- [CGAL](https://www.cgal.org/index.html)