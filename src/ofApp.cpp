#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup()
{
	float width = 600.f;
	float height = 350.f;
	float numCols = 35;
	float numRows = 20;
	float cellWidth = width / numCols;
	float cellHeight = height / numRows;

	for( int j = 0; j < numRows; j++ )
	{
		for( int i = 0; i < numCols; i++ )
		{
			points.emplace_back(
				i * cellWidth - width / 2.f + ofGetWidth() / 4.f,
				j * cellHeight - height / 2.f + ofGetHeight() / 4.f
			);
			original.addVertex( points.back() );
		}
	}

	for( int j = 0; j < numRows - 1; j++ )
	{
		for( int i = 0; i < numCols - 1; i++ )
		{
			original.addIndex( ( j + 0 ) * numCols + ( i + 0 ) );
			original.addIndex( ( j + 0 ) * numCols + ( i + 1 ) );
			original.addIndex( ( j + 1 ) * numCols + ( i + 1 ) );
			original.addIndex( ( j + 0 ) * numCols + ( i + 0 ) );
			original.addIndex( ( j + 1 ) * numCols + ( i + 1 ) );
			original.addIndex( ( j + 1 ) * numCols + ( i + 0 ) );
		}
	}

	PolyhedronBuilder<HalfedgeDS> builder( original );
	polyhedron1.delegate( builder );
	polyhedron2.delegate( builder );
	polyhedron3.delegate( builder );
	set_halfedgeds_items_id( polyhedron1 );
	set_halfedgeds_items_id( polyhedron2 );
	set_halfedgeds_items_id( polyhedron3 );

	deformator1 = new SurfaceMeshDeformation1( polyhedron1 );
	deformator2 = new SurfaceMeshDeformation2( polyhedron2 );
	deformator3 = new SurfaceMeshDeformation3( polyhedron3 );
	deformator3->set_sre_arap_alpha( 0.2 );

	// Definition of the region of interest (use the whole mesh)
	vertex_iterator vb, ve;
	boost::tie( vb, ve ) = CGAL::vertices( polyhedron1 );
	deformator1->insert_roi_vertices( vb, ve );
	boost::tie( vb, ve ) = CGAL::vertices( polyhedron2 );
	deformator2->insert_roi_vertices( vb, ve );
	boost::tie( vb, ve ) = CGAL::vertices( polyhedron3 );
	deformator3->insert_roi_vertices( vb, ve );

	// Create of mesh to reflect the deformation
	auto initDeformed = []( Polyhedron & polyhedron, ofMesh & mesh )
	{
		map< Kernel::Point_3 *, int > pointIndices;
		int count = 0;

		for( auto it = polyhedron.vertices_begin(); it != polyhedron.vertices_end(); ++it )
		{
			auto & p = it->point();
			mesh.addVertex( ofVec3f( p.x(), p.y(), p.z() ) );
			pointIndices[ &p ] = count++;
		}
		for( auto it = polyhedron.facets_begin(); it != polyhedron.facets_end(); ++it )
		{
			mesh.addIndex( pointIndices[ & it->halfedge()->vertex()->point() ] );
			mesh.addIndex( pointIndices[ & it->halfedge()->next()->vertex()->point() ] );
			mesh.addIndex( pointIndices[ & it->halfedge()->prev()->vertex()->point() ] );
		}
	};

	initDeformed( polyhedron1, deformed1 );
	initDeformed( polyhedron2, deformed2 );
	initDeformed( polyhedron3, deformed3 );

	deformator4.setup( original );

	ofSetBackgroundColor( 40 );
}

//--------------------------------------------------------------
void ofApp::update()
{
	if( deformedNeedUpdate )
	{
		deformator1->deform();
		deformator2->deform();
		deformator3->deform();
		deformator4.update();
		updateDeformedMesh();
		deformedNeedUpdate = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw()
{
	switch( state )
	{
		case ofApp::ADD_CTRL_POINTS:
			{
				ofSetColor( 200 );
				original.drawWireframe();
				ofSetColor( 0, 255, 0 );
				for( int i : ctrlPoints )
				{
					ofVec2f & p = points[ i ];
					ofDrawCircle( p.x, p.y, 3.f );
				}
				ofNoFill();
				ofDrawCircle( ofGetMouseX(), ofGetMouseY(), radius );
				ofFill();

				ofSetColor( 255 ); ofDrawBitmapString( "> [A]dd control points"   , 10.f, 20.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [R]emove control points", 10.f, 35.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [D]eform mesh"          , 10.f, 50.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [V]iew in 3D"           , 10.f, 65.f );
			}
			break;
		case ofApp::REMOVE_CTRL_POINTS:
			{
				ofSetColor( 200 );
				original.drawWireframe();
				ofSetColor( 255, 0, 0 );
				for( int i : ctrlPoints )
				{
					ofVec2f & p = points[ i ];
					ofDrawCircle( p.x, p.y, 3.f );
				}
				ofNoFill();
				ofDrawCircle( ofGetMouseX(), ofGetMouseY(), radius );
				ofFill();

				ofSetColor( 180 ); ofDrawBitmapString( "  [A]dd control points"   , 10.f, 20.f );
				ofSetColor( 255 ); ofDrawBitmapString( "> [R]emove control points", 10.f, 35.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [D]eform mesh"          , 10.f, 50.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [V]iew in 3D", 10.f, 65.f );
			}
			break;
		case ofApp::DEFORM:
			{

				auto ddraw = [ & ]( const ofMesh & mesh )
				{
					ofSetColor( 200 );
					mesh.drawWireframe();

					ofSetColor( 0, 0, 255 );
					for( int i : ctrlPoints )
					{
						const ofVec3f & p = mesh.getVertex( i );
						float r = 2.f;
						if( ofContains( draggedCtrlPoints, i ) ) r = 4.f;
						ofDrawCircle( p, r );
					}
				};

				ofPushMatrix();
				{
					ddraw( deformed1 );
					ofSetColor( 255, 0, 0 );
					ofDrawBitmapStringHighlight( "CGAL::ORIGINAL_ARAP", 50.f, 420.f, ofColor::black, ofColor::red );

					ofTranslate( ofGetWidth() / 2.f, 0.f );
					ddraw( deformed2 );
					ofDrawBitmapStringHighlight( "CGAL::SPOKES_AND_RIMS", 50.f, 420.f, ofColor::black, ofColor::red );

					ofTranslate( - ofGetWidth() / 2.f, ofGetHeight() / 2.f );
					ddraw( deformed3 );
					ofSetColor( 255, 0, 0 );
					ofDrawBitmapStringHighlight( "CGAL::SRE_ARAP", 50.f, 420.f, ofColor::black, ofColor::red );

					ofTranslate( ofGetWidth() / 2.f, 0.f );

					ofSetColor( 200 );
					deformator4.drawWireframe();
					ofSetColor( 0, 0, 255 );
					for( int i : ctrlPoints )
					{
						const ofVec3f & p = deformator4.getDeformedMesh().getVertex( i );
						float r = 2.f;
						if( ofContains( draggedCtrlPoints, i ) ) r = 4.f;
						ofDrawCircle( p, r );
					}

					ofSetColor( 255, 0, 0 );
					ofDrawBitmapStringHighlight( "ofxPuppet", 50.f, 420.f, ofColor::black, ofColor::red );
				}
				ofPopMatrix();

				ofNoFill();
				ofSetColor( 0, 0, 255 );
				ofDrawCircle( ofGetMouseX(), ofGetMouseY(), radius );
				ofFill();

				ofSetColor( 180 ); ofDrawBitmapString( "  [A]dd control points", 10.f, 20.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [R]emove control points", 10.f, 35.f );
				ofSetColor( 255 ); ofDrawBitmapString( "> [D]eform mesh", 10.f, 50.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [V]iew in 3D", 10.f, 65.f );
			}
			break;
		case ofApp::VIEW:
			{
				ofSetColor( 255 );
				cam.begin();
				ofPushMatrix();
				{
					ofTranslate( - ofGetWidth() / 2.f, - ofGetHeight() / 2.f, 0.f );
					deformed1.drawWireframe();
					ofTranslate( ofGetWidth() / 2.f, 0.f );
					deformed2.drawWireframe();
					ofTranslate( - ofGetWidth() / 2.f, ofGetHeight() / 2.f );
					deformed3.drawWireframe();
					ofTranslate( ofGetWidth() / 2.f, 0.f );
					deformator4.drawWireframe();
				}
				ofPopMatrix();
				cam.end();

				ofSetColor( 180 ); ofDrawBitmapString( "  [A]dd control points", 10.f, 20.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [R]emove control points", 10.f, 35.f );
				ofSetColor( 180 ); ofDrawBitmapString( "  [D]eform mesh", 10.f, 50.f );
				ofSetColor( 255 ); ofDrawBitmapString( "> [V]iew in 3D", 10.f, 65.f );
			}
			break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key)
{
	switch( key )
	{
		case 'a':
			state = ADD_CTRL_POINTS;
			break;
		case 'r':
			state = REMOVE_CTRL_POINTS;
			break;
		case 'd':
			state = DEFORM;
			initDeformation();
			break;
		case 'v':
			state = VIEW;
			break;
	}

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y )
{
	switch( state )
	{
		case ofApp::DEFORM:
			draggedCtrlPoints = getDeformedCtrlPointsAt( x, y, radius );
			break;
		default:
			break;
	}

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button)
{
	switch( state )
	{
		case ofApp::ADD_CTRL_POINTS:
			addControlPoints( x, y, radius );
			break;
		case ofApp::REMOVE_CTRL_POINTS:
			removeControlPoints( x, y, radius );
			break;
		case ofApp::DEFORM:
			deform( draggedCtrlPoints, x - ofGetPreviousMouseX(), y - ofGetPreviousMouseY() );
			break;
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button)
{
	switch( state )
	{
		case ofApp::ADD_CTRL_POINTS:
			addControlPoints( x, y, radius );
			break;
		case ofApp::REMOVE_CTRL_POINTS:
			removeControlPoints( x, y, radius );
			break;
		case ofApp::DEFORM:
			draggedCtrlPoints = getDeformedCtrlPointsAt( x, y, radius );
			break;
		default:
			break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button)
{
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

void ofApp::mouseScrolled( int x, int y, float scrollX, float scrollY )
{
	radius += scrollY * 3.f;
	if( radius < 3.f ) radius = 3.f;
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

void ofApp::addControlPoints( float x, float y, float radius )
{
	auto indices = getPointsAt( x, y, radius );
	for( int i : indices )
	{
		if( find( ctrlPoints.begin(), ctrlPoints.end(), i ) == ctrlPoints.end() )
		{
			ctrlPoints.push_back( i );
		}
	}
	deformedNeedUpdate = true;
}

void ofApp::removeControlPoints( float x, float y, float radius )
{
	auto indices = getPointsAt( x, y, radius );
	for( int i : indices )
	{
		auto it = find( ctrlPoints.begin(), ctrlPoints.end(), i );
		if( it != ctrlPoints.end() )
		{
			ctrlPoints.erase( it );
		}
	}
	deformedNeedUpdate = true;
}

vector<int> ofApp::getPointsAt( float x, float y, float radius )
{
	vector< int > indices;
	float rr = radius * radius;
	ofVec2f c( x, y );
	int i = 0;
	for( ofVec2f & p : points )
	{
		if( p.distanceSquared( c ) <= rr ) indices.push_back( i );
		i++;
	}
	return indices;
}

vector<int> ofApp::getDeformedCtrlPointsAt( float x, float y, float radius )
{
	vector<int> indices;

	vertex_iterator vb = polyhedron1.vertices_begin();
	float rr = radius * radius;
	ofVec2f c( x, y );

	for( auto i : ctrlPoints )
	{
		auto p1 = (* CGAL::cpp11::next( vb, i ))->point();
		ofVec2f p2( p1.x(), p1.y() );
		if( p2.distanceSquared( c ) <= rr ) indices.push_back( i );
	}

	return indices;
}

void ofApp::initDeformation()
{
	draggedCtrlPoints.clear();
	deformator1->clear_control_vertices();
	deformator1->reset();
	deformator2->clear_control_vertices();
	deformator2->reset();
	deformator3->clear_control_vertices();
	deformator3->reset();
	deformator4.clearControlPoints();
	deformedNeedUpdate = true;

	vertex_iterator vb = polyhedron1.vertices_begin();
	for( auto i : ctrlPoints ) deformator1->insert_control_vertex( * CGAL::cpp11::next( vb, i ) );
	vb = polyhedron2.vertices_begin();
	for( auto i : ctrlPoints ) deformator2->insert_control_vertex( * CGAL::cpp11::next( vb, i ) );
	vb = polyhedron3.vertices_begin();
	for( auto i : ctrlPoints ) deformator3->insert_control_vertex( * CGAL::cpp11::next( vb, i ) );
	
	for( auto i : ctrlPoints ) deformator4.setControlPoint( i );

	// The definition of the ROI and the control vertices is done, call preprocess
	bool is_matrix_factorization_OK = deformator1->preprocess();
	is_matrix_factorization_OK = is_matrix_factorization_OK && deformator2->preprocess();
	is_matrix_factorization_OK = is_matrix_factorization_OK && deformator3->preprocess();
	if( !is_matrix_factorization_OK )
	{
		ofLogError() << "Error in preprocessing, check documentation of preprocess()";
	}
}

template< class Deformator >
void deform2(
	Deformator * deformator, ofApp::Polyhedron polyhedron,
	const vector<int>& draggedCtrlPoints, float dx, float dy
)
{
	ofApp::vertex_iterator vb = polyhedron.vertices_begin();
	ofApp::Vector_3 translation( dx, dy, 0.0 );

	for( auto i : draggedCtrlPoints )
	{
		ofApp::vertex_descriptor control = * CGAL::cpp11::next( vb, i );
		ofApp::Point_3 newPos( control->point() );
		newPos += translation;
		deformator->set_target_position( control, newPos );
	}
};

void ofApp::deform( const vector<int>& draggedCtrlPoints, float dx, float dy )
{
	if( draggedCtrlPoints.empty() ) return;

	deform2( deformator1, polyhedron1, draggedCtrlPoints, dx, dy );
	deform2( deformator2, polyhedron2, draggedCtrlPoints, dx, dy );
	deform2( deformator3, polyhedron3, draggedCtrlPoints, dx, dy );

	for( auto i : draggedCtrlPoints )
	{
		const ofVec3f & v = deformator4.getDeformedMesh().getVertex( i );
		deformator4.setControlPoint( i, ofVec2f( v.x + dx, v.y + dy ) );
	}

	deformedNeedUpdate = true;
}

void ofApp::updateDeformedMesh()
{
	if( ! deformedNeedUpdate ) return;

	auto updte = []( Polyhedron & polyhedron, ofMesh & mesh )
	{
		int i = 0;
		for( auto it = polyhedron.vertices_begin(); it != polyhedron.vertices_end(); ++it )
		{
			auto & p = it->point();
			mesh.setVertex( i, ofVec3f( p.x(), p.y(), p.z() ) );
			i++;
		}
	};

	updte( polyhedron1, deformed1 );
	updte( polyhedron2, deformed2 );
	updte( polyhedron3, deformed3 );
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
