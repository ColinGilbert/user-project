#include "Application.hpp"
#include "Font.hpp"

std::unique_ptr<std::string> message_profiling, message_collision;
std::string test_message;


noob::actor_handle cannon_h;
noob::versorf cannon_orientation = noob::versor_from_axis_deg<float>(90, 1, 0.4, 0.4);
noob::prop_handle cannonball_h;



noob::font example_font;

noob::prop_handle wrecking_ball_h;
noob::scenery_handle wrecking_ball_fixed_to;

noob::prop_blueprints_handle ball_bp_h;  
noob::prop_blueprints ball_bp;

const size_t max_balls = 10;



void create_paddle_wheel(const noob::vec3f & pos, noob::stage & stage, noob::globals & g)
{
	float wheel_height = 100.0;
	float rotator_length = 200.0;
	// support 1
	stage.create_scenery(g.box_shape(10.0, wheel_height - 20.0, 10.0), noob::vec3f(pos.v[0], pos.v[1], pos.v[2] + rotator_length * 0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(10.0, wheel_height + 5.0, 10.0), noob::vec3f(pos.v[0] + 10.0, pos.v[1], pos.v[2] + rotator_length * 0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(10.0, wheel_height + 5.0, 10.0), noob::vec3f(pos.v[0] - 10.0, pos.v[1], pos.v[2] + rotator_length * 0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(30.0, 5.0, 10.0), noob::vec3f(pos.v[0], pos.v[1] + wheel_height - 46.0, pos.v[2] + rotator_length * 0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));

	// support 2 
	stage.create_scenery(g.box_shape(10.0, wheel_height - 20.0, 10.0), noob::vec3f(pos.v[0], pos.v[1], pos.v[2] + rotator_length * -0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(10.0, wheel_height + 5.0, 10.0), noob::vec3f(pos.v[0] + 10.0, pos.v[1], pos.v[2] + rotator_length * -0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(10.0, wheel_height + 5.0, 10.0), noob::vec3f(pos.v[0] - 10.0, pos.v[1], pos.v[2] + rotator_length * -0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));
	stage.create_scenery(g.box_shape(30.0, 5.0, 10.0), noob::vec3f(pos.v[0], pos.v[1] + wheel_height - 46.0, pos.v[2] + rotator_length * -0.25), noob::versorf(0.0, 0.0, 0.0, 1.0));

	//Rotator shaft	
	std::vector<noob::prop_handle> props_for_assembly;
	noob::prop_blueprints rotator_bp;
	rotator_bp.mass = 10.0;
	rotator_bp.shape = g.cylinder_shape(2.0, rotator_length, 12);//box_shape(10.0, 10.0, 100.0);
	rotator_bp.model = g.model_from_shape(rotator_bp.shape, 1);
	rotator_bp.reflect = g.get_reflectance("default");
	rotator_bp.ccd = true;
	noob::prop_blueprints_handle rotator_bp_h = stage.add_props_blueprints(rotator_bp);
	stage.reserve_props(rotator_bp_h, 1);
	noob::prop_handle rotator_h = stage.create_prop(rotator_bp_h, 0, pos, noob::versorf(0.0, 0.0, 0.0, 1.0));
	props_for_assembly.push_back(rotator_h);

	const size_t paddle_count = 10;
	// Create paddles
	noob::prop_blueprints paddle_bp;
	paddle_bp.mass = 10.0;
	paddle_bp.shape = g.box_shape(25.0, 1.0, 25.0);
	paddle_bp.model = g.model_from_shape(paddle_bp.shape, paddle_count);
	paddle_bp.reflect = g.get_reflectance("default");
	paddle_bp.ccd = true;
	noob::prop_blueprints_handle paddle_bp_h = stage.add_props_blueprints(paddle_bp);
	stage.reserve_props(paddle_bp_h, paddle_count);

	// Stoppers (So that the chain won't slip off)
	noob::prop_blueprints stoppers_bp;
	stoppers_bp.mass = 1.0;
	stoppers_bp.shape = g.cylinder_shape(10.0, 5.0, 12);
	stoppers_bp.model = g.model_from_shape(stoppers_bp.shape, 4);
	stoppers_bp.reflect = g.get_reflectance("default");
	stoppers_bp.ccd = true;
	noob::prop_blueprints_handle stoppers_bp_h = stage.add_props_blueprints(stoppers_bp);
	stage.reserve_props(stoppers_bp_h, 4);

	props_for_assembly.push_back(stage.create_prop(stoppers_bp_h, 0, pos + noob::vec3f(0.0, 0.0, 95.0), noob::versorf(0.0,0.0, 0.0, 1.0)));
	props_for_assembly.push_back(stage.create_prop(stoppers_bp_h, 0, pos + noob::vec3f(0.0, 0.0, 80.0), noob::versorf(0.0,0.0, 0.0, 1.0)));
	props_for_assembly.push_back(stage.create_prop(stoppers_bp_h, 0, pos + noob::vec3f(0.0, 0.0, -80.0), noob::versorf(0.0,0.0, 0.0, 1.0)));
	props_for_assembly.push_back(stage.create_prop(stoppers_bp_h, 0, pos + noob::vec3f(0.0, 0.0, -95.0), noob::versorf(0.0,0.0, 0.0, 1.0)));

	// Generate a unit circle on XY-plane
	const float PI = 3.1415926f;
	const float sector_step = 2 * PI / static_cast<float>(paddle_count);

	std::vector<noob::vec3f> unit_circle_vertices;
	for(int i = 0; i < paddle_count; ++i)
	{
		float sector_angle = i * sector_step;
		unit_circle_vertices.push_back(noob::vec3f(std::cos(sector_angle), std::sin(sector_angle), 0.0));
	}

	for(uint32_t j = 0; j < paddle_count; ++j)
	{
		const float sector_angle = j * sector_step;
		const noob::vec3f normal = unit_circle_vertices[j];
		const float ux = normal.v[0];
		const float uy = normal.v[1];

		//const noob::versorf orient(0.0, 0.0, 0.0, 1.0);
		const noob::versorf orient = noob::versor_from_axis_rad<float>(sector_angle, 1.0, 0.0, 0.0);
		const noob::vec3f position = pos + noob::vec3f(ux * 12.5, uy * 12.5, 0.0);
		noob::prop_handle paddle_h = stage.create_prop(paddle_bp_h, 0, position, orient);
		// noob::prop_handle paddle_h = stage.create_prop(paddle_bp_h, 0, noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
		props_for_assembly.push_back(paddle_h);
	}

	noob::prop_blueprints hitter_bp; //Lives at the ends of the paddlewheel and actuates something else
	hitter_bp.mass = 10.0;
	hitter_bp.shape = g.box_shape(10., 180., 10.0);
	hitter_bp.model = g.model_from_shape(hitter_bp.shape, 2);
	hitter_bp.reflect = g.get_reflectance("default");
	hitter_bp.ccd = true;
	
	noob::prop_blueprints_handle hitter_bp_h = stage.add_props_blueprints(hitter_bp);
	stage.reserve_props(hitter_bp_h, 2);	

	props_for_assembly.push_back(stage.create_prop(hitter_bp_h, 0, pos + noob::vec3f(0.0, 0.0, -90.0), noob::versorf(0.0,0.0, 0.0, 1.0)));
	props_for_assembly.push_back(stage.create_prop(hitter_bp_h, 0, pos + noob::vec3f(0.0, 0.0, 90.0), noob::versorf(0.0,0.0, 0.0, 1.0)));

	noob::assembly_handle assembly_h = stage.create_assembly(noob::vec3f(0.0, wheel_height * 0.5, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0), true, props_for_assembly);

	noob::body b = stage.get_body(stage.get_body_handle(assembly_h));
	b.set_angular_factor(noob::vec3f(0.0, 0.0, 1.0));
	b.set_linear_factor(noob::vec3f(0.0, 0.0, 0.0));
	// Add constraint for rotator
	//	noob::constraint_handle hinge_h = stage.create_hinge_constraint(stage.get_body_handle(assembly_h), noob::vec3f(-100.0, 0, 0.0), noob::vec3f(0.0, 0.0, 1.0));

	// Chain for pulling rocket's lid off :)
			

	//size_t chain_links_max = 50;

	noob::actor_blueprints cannon_bp;
	cannon_bp.bounds = g.cylinder_shape(10.0, 60.0, 12);
	cannon_bp.model = g.model_from_shape(cannon_bp.bounds, 1);
	cannon_bp.reflect = g.get_reflectance("default");
	
	noob::actor_blueprints_handle cannon_bp_h = stage.add_actor_blueprints(cannon_bp);
	stage.reserve_actors(cannon_bp_h, 1);
	cannon_h = stage.create_actor(cannon_bp_h, 0, pos + noob::vec3f(90.0, 0.0, -100.0), cannon_orientation);


	// Boulder
	noob::prop_blueprints cannonball_bp;
	cannonball_bp.mass = 100.0;
	cannonball_bp.shape = g.sphere_shape(10.0);
	cannonball_bp.model = g.model_from_shape(cannonball_bp.shape, 1);
	cannonball_bp.ccd = true;
	cannonball_bp.reflect = g.get_reflectance("default");

	noob::prop_blueprints_handle cannonball_bp_h = stage.add_props_blueprints(cannonball_bp);
	stage.reserve_props(cannonball_bp_h, 1);
	cannonball_h = stage.create_prop(cannonball_bp_h, 0, pos + noob::vec3f(115.0, 10.0, -145.0), noob::versorf(0.0, 0.0 ,0.0 ,1.0));
}


void create_wrecking_ball(const noob::vec3f &pos, size_t chain_length, noob::stage &stage_arg, noob::globals &g)
{
	noob::scenery_handle fixture_h = stage_arg.create_scenery(g.box_shape(1.0, 1.0, 1.0), pos, noob::versorf(0.0, 0.0, 0.0, 1.0));

	noob::prop_blueprints chain_link_bp;
	chain_link_bp.mass = 5.0;
	chain_link_bp.shape = g.sphere_shape(0.4);
	chain_link_bp.model = g.model_from_shape(chain_link_bp.shape, chain_length);
	chain_link_bp.reflect = g.get_reflectance("default");
	chain_link_bp.ccd = false;

	noob::prop_blueprints_handle chain_link_h = stage_arg.add_props_blueprints(chain_link_bp);
	stage_arg.reserve_props(chain_link_h, chain_length);
	noob::body_handle last_bod = stage_arg.get_scenery(fixture_h).body;

	float y_accum = pos.v[1];
	for (uint32_t i = 0; i < chain_length; ++i)
	{
		y_accum -= 1.0;

		noob::prop_handle ph = stage_arg.create_prop(chain_link_h, 0, noob::vec3f(pos.v[0], y_accum, pos.v[2]), noob::versorf(0.0, 0.0, 0.0, 1.0));

		noob::body_handle current_bod = stage_arg.get_prop(ph).body;
		//noob::constraint_handle twisty_h = stage.create_point_constraint(current_bod, last_bod, noob::identity_mat4<float>(), noob::identity_mat4<float>());

		noob::constraint_handle twisty_h = stage_arg.create_point_constraint(current_bod, last_bod, noob::vec3f(0.0, -0.5, 0.0), noob::vec3f(0.0, 0.5, 0.0));

		//noob::constraint twisty = stage.get_constraint(twisty_h);
		last_bod = current_bod;
	}

	noob::shape_handle large_sphere_shp = g.sphere_shape(7.0);
	noob::prop_blueprints wrecking_ball_bp;
	wrecking_ball_bp.mass = 100.0;
	wrecking_ball_bp.shape = large_sphere_shp;
	wrecking_ball_bp.model = g.model_from_shape(large_sphere_shp, 1);
	wrecking_ball_bp.reflect = g.get_reflectance("default");
	wrecking_ball_bp.ccd= true;

	auto wrecking_ball_bp_h = stage_arg.add_props_blueprints(wrecking_ball_bp);
	stage_arg.reserve_props(wrecking_ball_bp_h, 1);

	wrecking_ball_h = stage_arg.create_prop(wrecking_ball_bp_h, 0, noob::vec3f(200, -50, -100), noob::versorf(0.0, 0.0, 0.0, 1.0));
	auto wrecking_ball_bod = stage_arg.get_prop(wrecking_ball_h).body;
	//auto constrain_h = stage.create_conical_constraint(wrecking_ball_bod, last_bod, noob::translate(noob::identity_mat4<float>(), noob::vec3f(0.0, 7.0, 0.0)), noob::translate(noob::identity_mat4<float>(), noob::vec3f(0.0, -0.5, 0.0)));
	auto constrain_h = stage_arg.create_point_constraint(wrecking_ball_bod, last_bod, noob::vec3f(0.0, 7.0, 0.0), noob::vec3f(0.0, -0.5, 0.0));
}


void create_slide(noob::stage &stage, noob::globals &g)
{
	const float rim_height = 20.0;
	const float track_width = 40.0;
	const float wall_thickness = 1.0;
	const float track_thickness = 2.0;
	const float starting_z = -100.0;
	const float track_length_1 = 200.0;
	
	const noob::vec3f pos_1(100.0, -10.0, starting_z);
	const noob::versorf orient_1(0.0, 0.0, 0.0, 1.0);
	stage.create_scenery(g.box_shape(track_length_1, track_thickness, track_width), pos_1 + noob::vec3f(0.0, 0.0, track_width * 0.5), orient_1);
	stage.create_scenery(g.box_shape(track_length_1, rim_height, wall_thickness), pos_1 + noob::vec3f(0.0, rim_height * 0.5, track_width - wall_thickness * 0.5), orient_1);
	stage.create_scenery(g.box_shape(track_length_1, rim_height, wall_thickness), pos_1 + noob::vec3f(0.0, rim_height * 0.5, wall_thickness * 0.5), orient_1);
	
	const float track_length_2 = 200.0;
	const noob::vec3f pos_2(300.0, -20.0, starting_z); 
	const noob::versorf orient_2 = noob::versor_from_axis_deg<float>(30.0, 0.0, 0.3, 0.0);
	
	wrecking_ball_fixed_to = stage.create_scenery(g.box_shape(track_length_2, track_thickness, track_width), pos_2 + noob::vec3f(0.0, 0.0, track_width * 0.5), orient_2);
	stage.create_scenery(g.box_shape(track_length_2, rim_height, wall_thickness), pos_2 + noob::vec3f(0.0, track_thickness + rim_height*0.5, track_width - wall_thickness * 0.5), orient_2);
	stage.create_scenery(g.box_shape(track_length_2, rim_height, wall_thickness), pos_2 + noob::vec3f(0.0, track_thickness + rim_height*0.5, wall_thickness * 0.5), orient_2);




	
}


void create_brick_wall(const noob::vec3f &pos, const noob::vec3f &brick_dims, float brick_mass, const noob::vec2f &bricks_count, noob::stage &stage, noob::globals &g)
{
	const size_t bricks_total = bricks_count.v[0] * bricks_count.v[1];
	const size_t x_count = bricks_count.v[0];
	const size_t y_count = bricks_count.v[1];

	const noob::versorf orient = noob::versorf(0., 0., 0., 1.);
	// Create brick template
	noob::prop_blueprints brick_bp;
	brick_bp.mass = 5.0;
	brick_bp.shape = g.box_shape(brick_dims.v[0], brick_dims.v[1], brick_dims.v[2]);
	brick_bp.model = g.model_from_shape(brick_bp.shape, bricks_total);
	brick_bp.reflect = g.get_reflectance("default");
	brick_bp.ccd = false;

	// Create floor
	noob::scenery_handle floor = stage.create_scenery(g.box_shape(brick_dims.v[0] * x_count, 1.0, brick_dims.v[2]), pos + noob::vec3f(brick_dims.v[0] * x_count * 0.5 - brick_dims.v[0] * 0.5, 0.0, brick_dims.v[2]), orient);

	// Set them up!
	float y_offset = pos.v[1];
	// float x_offset = 0.0;

	noob::prop_blueprints_handle brick_bp_h = stage.add_props_blueprints(brick_bp);
	stage.reserve_props(brick_bp_h, bricks_total);
	// noob::body_handle last_bod = stage.create_prop(stage.get_scenery(brick_h).body;

	for (size_t yy = 0; yy < y_count; ++yy)
	{
		for (size_t xx = 0; xx < x_count; ++xx)
		{
			noob::prop_handle brick_h = stage.create_prop(brick_bp_h, 0, pos + noob::vec3f(static_cast<float>(static_cast<double>(xx) * static_cast<double>(brick_dims.v[0])) + 0.1, static_cast<float>(static_cast<double>(yy) * static_cast<double>((brick_dims.v[1]))), brick_dims.v[2]), orient);
			// noob::body_handle current_bod = stage.get_prop(ph).body;
			// noob::constraint_handle pc = stage.create_point_constraint(current_bod, last_bod, noob::vec3f(0.0, 0.5, 0.0), noob::vec3f(0.0, -0.5, 0.0));
			// y_accum -= 1.0;
			// last_bod = current_bod;
		}
	}
}

/*
   void create_trebuchet(const noob::vec3f &pos, const noob::versorf &orient, noob::stage &stage, noob::globals &g)
   {
// Create frame
// noob::scenery_handle base = stage.create_scenery(g.box_shape(10.0, 1.0, 100.0), noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
noob::scenery_handle pivot_beam = stage.create_scenery(g.cylinder_shape(1.0, 1.0, 12), pos, orient);

noob::prop_blueprints throwing_arm_bp;
throwing_arm_bp.mass = 10.0;
throwing_arm_bp.shape = g.box_shape(2.0, 2.0, 100.0);
throwing_arm_bp.model = g.model_from_shape(throwing_arm_bp.shape, 1);
throwing_arm_bp.reflect = g.get_reflectance("default");
throwing_arm_bp.ccd = false;
noob::prop_blueprints_handle throwing_arm_bp_h = stage.add_props_blueprints(throwing_arm_bp);
stage.reserve_props(throwing_arm_bp_h, 1);
const noob::prop_handle arm_h = stage.create_prop(throwing_arm_bp_h, 0, pos + noob::vec3f(0.0, 5.2, 30.0), orient * noob::versor_from_axis_deg<float>(90, 0.0, 1.0, 0.0));
stage.get_body(stage.get_prop(arm_h).body).set_angular_factor(noob::vec3f(1.0, 0.0, 0.0));
stage.get_body(stage.get_prop(arm_h).body).set_linear_factor(noob::vec3f(0.0, 0.0, 0.0));

stage.create_hinge_constraint(stage.get_prop(arm_h).body, stage.get_scenery(pivot_beam).body, noob::vec3f(0.0, 0.0, 0.0), noob::vec3f(0.0, 0.0, 0.0), noob::vec3f(1.0, 1.0, 1.0), noob::vec3f(1.0, 1.0, 1.0));

noob::prop_blueprints boulder_bp;
boulder_bp.mass = 100.0;
boulder_bp.shape = g.sphere_shape(5.0);
boulder_bp.model = g.model_from_shape(boulder_bp.shape, 1);
boulder_bp.reflect = g.get_reflectance("default");
boulder_bp.ccd = false;
noob::prop_blueprints_handle boulder_bp_h = stage.add_props_blueprints(boulder_bp);
stage.reserve_props(boulder_bp_h, 1);
const noob::prop_handle boulder_h = stage.create_prop(boulder_bp_h, 0, pos, orient);

stage.create_point_constraint(stage.get_prop(arm_h).body, stage.get_prop(boulder_h).body, noob::vec3f(0.0, 0.0, 60.0), noob::vec3f(0.0, 0.0, 0.0));	


noob::scenery_handle rotator_target_h = stage.create_scenery(g.box_shape(1.0, 1.0, 1.0), noob::vec3f(0.0, -50.0, 20.0), noob::versorf(0.0, 0.0, 0.0, 1.0));


stage.create_point_constraint(stage.get_prop(arm_h).body, stage.get_scenery(rotator_target_h).body, noob::vec3f(0.0, 0.0, 50.0), noob::vec3f(0.0, 0.0, 0.0));	
// noob::prop_blueprints cup_bottom_bp;
// cup_bottom_bp.mass = 1.0;
// cup_bottom_bp.shape = g.box_shape(10.0, 1.0, 10.0);
// cup_bottom_bp.model = g.model_from_shape(cup_bottom.shape);
// cup_bottom_bp.reflect = g.get_reflectance("default");
// cup_bottom_bp.ccd = false;


// Create pivot-joint
// noob::constraint_handle pivot_h = stage.create_hinge_constraint();
}	
 */	


void create_rocket(const noob::vec3f & pos, const noob::versorf & orient)
{
	// Make long cylinder, (and boosters?)

	// Make capped cones for top to make it look real

	// Make stabilizers (less important than the rest, as they are purely decorative)

	// Add triggers for firing
}

/*
void create_cannon(const noob::vec3f & pos, noob::stage & stage, noob::globals & g)
o
{
	const noob::versorf orient = noob::versorf(0., 0., 0., 1.);
	const float PI = 3.1415926;
	const float bore_radius = 10.0;
	const float cannon_length = 30.0;	
	const float segment_radius = 1.5;
	const size_t segment_count = 12;
	const float sector_step = 2.0 * PI / (static_cast<float>(segment_count));
	const float cap_thickness = 5.0;

	float sector_angle; 
	
	std::vector<noob::prop_handle> cannon_parts; 
	// Cylindrical segments
	noob::prop_blueprints segment_bp;
	segment_bp.mass = 100.0;
	segment_bp.shape = g.cylinder_shape(segment_radius, cannon_length, 12);
	segment_bp.model = g.model_from_shape(segment_bp.shape, 12);
	segment_bp.reflect = g.get_reflectance("default");
	segment_bp.ccd = true;

	noob::prop_blueprints_handle segment_bp_h = stage.add_props_blueprints(segment_bp);
	stage.reserve_props(segment_bp_h, segment_count);
	for (size_t i = 0; i < segment_count; ++i)
	{
		sector_angle = static_cast<float>(i) * sector_step;
		cannon_parts.push_back(stage.create_prop(segment_bp_h, 0, pos + noob::vec3f(bore_radius * std::cos(sector_angle), bore_radius * std::sin(sector_angle), 0.0), noob::versorf(0.0, 0., 0., 1.)));
	}

	// Cap the non-firing end of the cannon with a cylinder of appropriate width
	noob::prop_blueprints cap_bp;
	cap_bp.mass = 500.0;
	cap_bp.shape = g.cylinder_shape(bore_radius, cap_thickness, 12);
	cap_bp.model = g.model_from_shape(cap_bp.shape, 1);
	cap_bp.reflect = g.get_reflectance("default");
	cap_bp.ccd = true;

	noob::prop_blueprints_handle cap_bp_h = stage.add_props_blueprints(cap_bp);
	stage.reserve_props(cap_bp_h, 1);
	cannon_parts.push_back(stage.create_prop(cap_bp_h, 0, pos + noob::vec3f(0., 0., (cannon_length - cap_thickness)), orient));

	cannon_h = stage.create_assembly(pos, orient, true, cannon_parts);



	// Add a big ole' cannonball

	// Add triggers for firing
}
*/

bool noob::application::user_init()
{
	message_profiling = std::make_unique<std::string>("");
	message_collision = std::make_unique<std::string>("");
	noob::logger::log(noob::importance::INFO, "[Application] Begin user init!");

	noob::globals& g = noob::get_globals();

	// const noob::shape_handle scenery_shp = g.cylinder_shape(10.0, 50.0, 12);
	// const noob::scenery_handle sc_h = stage.create_scenery(scenery_shp, noob::vec3f(0.0, -50.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));

	noob::reflectance r;
	r.set_specular(noob::vec3f(0.1, 0.1, 0.1));
	r.set_diffuse(noob::vec3f(0.1, 0.1, 0.1));
	r.set_emissive(noob::vec3f(0.1, 0.1, 0.1));
	r.set_shine(0.1);
	r.set_albedo(0.1);
	r.set_fresnel(0.1);

	const noob::reflectance_handle rh = g.reflectances.add(r);

	// const float actor_dims = 1.0;
	// const noob::shape_handle shp = g.sphere_shape(actor_dims);
	// const noob::shape_handle unit_sphere_shp = g.sphere_shape(0.5);
	// const uint32_t actor_count = 50;

	// create_wrecking_ball(noob::vec3f(0.0, 100.0, 0.0), 60, stage, noob::get_globals());

	// create_slide(noob::vec3f(50.0, -50.0, 50.0), noob::versorf(0.0, 0.0, 0.0, 1.0), 100.0, 15.0, 5.0, stage, noob::get_globals());

	// create_brick_wall(noob::vec3f(10.0, 70.0, -10.0), noob::versorf(0.0, 0.0, 0.0, 1.0), noob::vec3f(10.0, 4.0, 4.0), 10.0, noob::vec2f(30, 10), stage, noob::get_globals());//& brick_counts);

	// noob::vec2d ret_val = db.vec2fp_get(db_idx).value;
	// noob::logger::log(noob::importance::INFO, noob::concat("[Application] Inserted value: ", noob::to_string(test_val), ". Returned value: ", noob::to_string(ret_val), "."));

	// create_trebuchet(noob::vec3f(0.0, 0.0, 0.0), noob::versor_from_axis_deg<float>(90.0f, 0.0f, 1.0f, 0.0f), stage, noob::get_globals());

	// create_cannon(noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0), 100, stage, g);

	create_paddle_wheel(noob::vec3f(0.0, 50.0, 0.0), stage, g);

	noob::scenery_handle fixture_h = stage.create_scenery(g.box_shape(100.0, 1.0, 100.0), noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));

	ball_bp.mass = 1000.0;;
	ball_bp.shape = g.sphere_shape(0.5);
	ball_bp.model = g.model_from_shape(ball_bp.shape, max_balls);
	ball_bp.reflect = g.get_reflectance("default");;
	ball_bp.ccd = true;

	ball_bp_h = stage.add_props_blueprints(ball_bp);

	stage.reserve_props(ball_bp_h, max_balls);

	size_t ball_count = 0;


	while (ball_count < max_balls)
	{	
		noob::prop_handle ball_h = stage.create_prop(ball_bp_h, 0, noob::vec3f(-10.0, 230.0 + static_cast<float>(ball_count) * 12.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
		noob::body b = stage.get_body(stage.get_prop(ball_h).body);
		// b.set_linear_factor(noob::vec3f(0.0, 1.0, 0.0));
		++ball_count;

	}

	// b.set_linear_factor(noob::vec3f(1.0, 1.0, 0.0));
	//time_since_last_drop += ball_drop_interval;

//	create_wrecking_ball(noob::vec3f(150.0, 0.0, -200.0), 100, stage, g);
//
//	create_slide(stage, g);


//	noob::constraint_handle wrecking_ball_constraint_h = stage.create_fixed_constraint(stage.get_scenery(wrecking_ball_fixed_to).body, stage.get_prop(wrecking_ball_h).body, noob::translate(noob::identity_mat4<float>(), noob::vec3f(100.0, 0.0, 0.0)), noob::identity_mat4<float>());
//	noob::constraint wrecking_ball_constraint = stage.get_constraint(wrecking_ball_constraint_h);


	create_brick_wall(noob::vec3f(300., -440., -500.), noob::vec3f(10., 10., 10.), 5.0, noob::vec2f(30, 10), stage, g);





	logger::log(noob::importance::INFO, "[Application] Successfully done (C++) user init.");
	paused = false;

	return true;
}


void noob::application::user_update(double dt)
{
	// app_gui.text("Happy happy fun fun!!!!", noob::vec2f(100.0, 100.0), noob::gui::font_size::READING, noob::vec4f(1.0, 1.0, 0.0, 1.0));

	noob::globals& g = noob::get_globals();
	const noob::time_point nowtime = noob::clock::now();
	const noob::duration time_since_update = nowtime - last_ui_update;

	//	logger::log(noob::importance::INFO, "[UserApp] Dropping a ball");
	const uint64_t profiling_interval = 3000;

	noob::body b = stage.get_body(stage.get_prop(wrecking_ball_h).body);
	// TESTING b.apply_impulse(noob::vec3f(0.0, 0.0, 50.0), noob::vec3f(0.0, 0.0, -7.0));

	if (noob::millis(time_since_update) > profiling_interval - 1)
	{
		const noob::profiler_snap snap = g.profile_run;

		message_profiling = std::make_unique<std::string>(noob::concat("[User Update] Frame time: ", noob::to_string(divide_duration(snap.total_time, profiling_interval)), std::string(", draw time: "), noob::to_string(divide_duration(snap.stage_draw_duration, profiling_interval)), ", physics time: ", noob::to_string(divide_duration(snap.stage_physics_duration, profiling_interval))));

		noob::logger::log(noob::importance::INFO, *message_profiling);

		g.profile_run.total_time = g.profile_run.stage_physics_duration = g.profile_run.stage_draw_duration = g.profile_run.sound_render_duration = noob::duration(0);
		g.profile_run.num_sound_callbacks = 0;

		last_ui_update = nowtime;
	}
}
