#pragma once

#include "ofMain.h"
#define CGAL_EIGEN3_ENABLED
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Vector_3.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Surface_mesh_deformation.h>
#include "ofxPuppet.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void mouseScrolled( int x, int y, float scrollX, float scrollY );
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		void addControlPoints( float x, float y, float radius );
		void removeControlPoints( float x, float y, float radius );
		vector< int > getPointsAt( float x, float y, float radius );
		vector< int > getDeformedCtrlPointsAt( float x, float y, float radius );
		void initDeformation();
		void deform( const vector< int > & draggedCtrlPoints, float dx, float dy );
		void updateDeformedMesh();
		
		vector< ofVec2f > points;
		vector< int > ctrlPoints;
		vector< int > draggedCtrlPoints;

		ofMesh original;
		ofMesh deformed1;
		ofMesh deformed2;
		ofMesh deformed3;
		bool deformedNeedUpdate = true;

		ofEasyCam cam;

		enum States { ADD_CTRL_POINTS, REMOVE_CTRL_POINTS, DEFORM, VIEW };
		States state = States::ADD_CTRL_POINTS;

		float radius = 40;

		typedef CGAL::Simple_cartesian<double>Kernel;
		typedef CGAL::Vector_3<Kernel>                                       Vector_3;
		typedef CGAL::Point_3<Kernel>                                        Point_3;
		typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3> Polyhedron;
		typedef boost::graph_traits<Polyhedron>::vertex_descriptor           vertex_descriptor;
		typedef boost::graph_traits<Polyhedron>::vertex_iterator             vertex_iterator;
		typedef CGAL::Surface_mesh_deformation<Polyhedron, CGAL::Default, CGAL::Default, CGAL::ORIGINAL_ARAP > SurfaceMeshDeformation1;
		typedef CGAL::Surface_mesh_deformation<Polyhedron, CGAL::Default, CGAL::Default, CGAL::SPOKES_AND_RIMS > SurfaceMeshDeformation2;
		typedef CGAL::Surface_mesh_deformation<Polyhedron, CGAL::Default, CGAL::Default, CGAL::SRE_ARAP > SurfaceMeshDeformation3;
		typedef Polyhedron::HalfedgeDS                                       HalfedgeDS;

		Polyhedron polyhedron1;
		Polyhedron polyhedron2;
		Polyhedron polyhedron3;
		SurfaceMeshDeformation1 * deformator1;
		SurfaceMeshDeformation2 * deformator2;
		SurfaceMeshDeformation3 * deformator3;
		ofxPuppet deformator4;

		template<class HDS>
		class PolyhedronBuilder : public CGAL::Modifier_base<HDS>
		{
		public:
			ofMesh & mesh;
			PolyhedronBuilder( ofMesh & _mesh ) : mesh( _mesh ) {}
			void operator()( HDS& hds )
			{
				typedef typename HDS::Vertex   Vertex;
				typedef typename Vertex::Point Point;

				// create a cgal incremental builder
				CGAL::Polyhedron_incremental_builder_3<HDS> B( hds, true );
				B.begin_surface( mesh.getNumVertices(), mesh.getNumIndices() / 3 );

				// add the polyhedron vertices
				const auto & vertices = mesh.getVertices();
				for( const ofVec3f & v : vertices )
				{
					B.add_vertex( Point( v.x, v.y, v.z ) );
				}

				// add the polyhedron triangles
				const auto & indices = mesh.getIndices();
				for( size_t i = 0; i < indices.size(); i += 3 )
				{
					int i0 = indices[ i ];
					int i1 = indices[ i + 1 ];
					int i2 = indices[ i + 2 ];
					ofVec3f v1 = vertices[ i1 ] - vertices[ i0 ];
					ofVec3f v2 = vertices[ i2 ] - vertices[ i0 ];

					if( v1.angle( v2 ) > 0.f ) swap( i1, i2 );

					B.begin_facet();
					B.add_vertex_to_facet( i0 );
					B.add_vertex_to_facet( i1 );
					B.add_vertex_to_facet( i2 );
					B.end_facet();
				}

				// finish up the surface
				B.end_surface();
			}
		};

};
