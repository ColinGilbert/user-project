#include "Application.hpp"
#include "Font.hpp"

std::unique_ptr<std::string> message_profiling, message_collision;
std::string test_message;

noob::actor_handle ah;
noob::prop_handle ph;
noob::font example_font;




void create_paddle_wheel(const noob::vec3f & pos, noob::stage & stage, noob::globals & g)
{
	// Fixture and shaft

	noob::scenery_handle fixture_h = stage.create_scenery(g.box_shape(1.0, 1.0, 1.0), noob::vec3f(pos.v[0] + 0.0, pos.v[1] + 0.0, pos.v[2] + 110.0), noob::versorf(0.0, 0.0, 0.0, 1.0));



	noob::prop_blueprints rotator_bp;
	rotator_bp.mass = 10.0;
	rotator_bp.shape = g.cylinder_shape(10.0, 100.0, 12);//box_shape(10.0, 10.0, 100.0);
	rotator_bp.model = g.model_from_shape(rotator_bp.shape, 1);
	rotator_bp.reflect = g.get_reflectance("default");
	rotator_bp.ccd = true;

	noob::prop_blueprints_handle rotator_bp_h = stage.add_props_blueprints(rotator_bp);
	stage.reserve_props(rotator_bp_h, 1);

	noob::prop_handle rotator_h = stage.create_prop(rotator_bp_h, 0, pos, noob::versorf(0.0, 0.0, 0.0, 1.0));

	noob::body b = stage.get_body(stage.get_prop(rotator_h).body);
	b.set_angular_factor(noob::vec3f(0.0, 0.0, 1.0));
	b.set_linear_factor(noob::vec3f(0.0, 0.0, 0.0));
	// Add constraint between rotator and fixture
	noob::constraint_handle hinge_h = stage.create_hinge_constraint(stage.get_scenery(fixture_h).body, stage.get_prop(rotator_h).body, noob::vec3f(0.0, 0.0, -100.0), noob::vec3f(0.0, 0.0, 0.0), noob::vec3f(0.0, 0.0, 1.0), noob::vec3f(0.0, 0.0, 1.0));

	// Chain for pulling rocket's lid off :)




	// Create paddles
	const size_t paddle_count = 10;
	
	noob::prop_blueprints paddle_bp;
	paddle_bp.mass = 10.0;
	paddle_bp.shape = g.box_shape(25.0, 1.0, 25.0);
	paddle_bp.model = g.model_from_shape(paddle_bp.shape, paddle_count);
	paddle_bp.reflect = g.get_reflectance("default");
	paddle_bp.ccd = true;
 
	noob::prop_blueprints_handle paddle_bp_h = stage.add_props_blueprints(paddle_bp);
	stage.reserve_props(paddle_bp_h, paddle_count);

/*
	// Create paddle-shaft
	noob::prop_blueprints shaft_bp;
	shaft_bp.mass = 10.0;
	shaft_bp.shape = g.cylinder_shape(0.5, 50.0, 12);
	shaft_bp.model = g.model_from_shape(shaft_bp.shape);
	shaft_bp.reflect = g.get_reflectance("default");
	shaft_bp.ccd = false;
	
	noob::prop_blueprints_handle shaft_bp_h = stage.add_props_blueprints(shaft_bp);
	stage.reserve_props(shaft_bp, paddle_count);
*/

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
		const noob::versorf orient = noob::versor_from_axis_rad<float>(sector_angle, 0.0, 0.0, 1.0);
		const noob::vec3f position = noob::vec3f(ux * 42.5, uy * 42.5, 0.0);
		noob::prop_handle paddle_h = stage.create_prop(paddle_bp_h, 0, position, noob::versorf(0.0, 0.0, 0.0, 1.0));//orient);
		// noob::prop_handle paddle_h = stage.create_prop(paddle_bp_h, 0, noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
		noob::body b = stage.get_body(stage.get_prop(paddle_h).body);
		b.set_linear_factor(noob::vec3f(0.0, 0.0, 1.0));	
		// b.set_angular_factor(noob::vec3f(1.0, 0.0, 1.0));
		
		noob::constraint_handle fixed_h = stage.create_fixed_constraint(stage.get_prop(rotator_h).body, stage.get_prop(paddle_h).body, noob::translate(noob::rotate(noob::identity_mat4<float>(), orient), noob::vec3f(position.v[0], position.v[1], 0.0)), noob::identity_mat4<float>());//noob::rotate(noob::identity_mat4<float>(), orient));
		noob::constraint fixed = stage.get_constraint(fixed_h);
		for (size_t i = 0; i < 12; ++i)
		{
			fixed.set_stiffness(i, 1.0);
			fixed.set_bounce(i, 0.0);
			fixed.set_damping(i, 0.0);
		}		

	}
}


void create_wrecking_ball(const noob::vec3f &pos, size_t chain_length, noob::stage &stage, noob::globals &g)
{
	noob::scenery_handle fixture_h = stage.create_scenery(g.box_shape(1.0, 1.0, 1.0), pos, noob::versorf(0.0, 0.0, 0.0, 1.0));

	noob::prop_blueprints chain_link_bp;
	chain_link_bp.mass = 5.0;
	chain_link_bp.shape = g.sphere_shape(0.5);
	chain_link_bp.model = g.model_from_shape(chain_link_bp.shape, chain_length);
	chain_link_bp.reflect = g.get_reflectance("default");
	chain_link_bp.ccd = false;

	noob::prop_blueprints_handle chain_link_h = stage.add_props_blueprints(chain_link_bp);
	stage.reserve_props(chain_link_h, chain_length);
	noob::body_handle last_bod = stage.get_scenery(fixture_h).body;

	float y_accum = pos.v[1] - 0.5;
	for (uint32_t i = 0; i < chain_length; ++i)
	{
		ph = stage.create_prop(chain_link_h, 0, noob::vec3f(0.0, y_accum, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
		noob::body_handle current_bod = stage.get_prop(ph).body;
		noob::constraint_handle pc = stage.create_point_constraint(current_bod, last_bod, noob::vec3f(0.0, 0.5, 0.0), noob::vec3f(0.0, -0.5, 0.0)); 
		y_accum -= 1.0;
		last_bod = current_bod;
	}

	noob::shape_handle large_sphere_shp = g.sphere_shape(7.0);
	noob::prop_blueprints wrecking_ball_bp;
	wrecking_ball_bp.mass = 100.0;
	wrecking_ball_bp.shape = large_sphere_shp;
	wrecking_ball_bp.model = g.model_from_shape(large_sphere_shp, 1);
	wrecking_ball_bp.reflect = g.get_reflectance("default");
	wrecking_ball_bp.ccd= true;

	auto wrecking_ball_bp_h = stage.add_props_blueprints(wrecking_ball_bp);
	stage.reserve_props(wrecking_ball_bp_h, 1);

	auto wrecking_ball_h = stage.create_prop(wrecking_ball_bp_h, 0, noob::vec3f(0.0, y_accum, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));

	auto wrecking_ball_bod = stage.get_prop(wrecking_ball_h).body;
	auto constrain_h = stage.create_point_constraint(wrecking_ball_bod, last_bod, noob::vec3f(0.0, 7.0, 0.0), noob::vec3f(0.0, -0.5, 0.0));
}


void create_slide(const noob::vec3f &pos, const noob::versorf &orient, float length, float width, float rim_height, noob::stage &stage, noob::globals &g)
{
	// Create main feature
	noob::scenery_handle slide_base_h = stage.create_scenery(g.box_shape(width, 1.0, length), pos, orient);
	// Create rims
	noob::scenery_handle rim_1_h = stage.create_scenery(g.box_shape(1.0, rim_height, length), pos + noob::vec3f(1.0 + width * 0.5, 1.0, 0.0), orient);
	noob::scenery_handle rim_2_h = stage.create_scenery(g.box_shape(1.0, rim_height, length), pos + noob::vec3f(-1.0 + width * -0.5, 1.0, 0.0), orient);
}


void create_brick_wall(const noob::vec3f &pos, const noob::versorf &orient, const noob::vec3f &brick_dims, float brick_mass, const noob::vec2f &bricks_count, noob::stage &stage, noob::globals &g)
{
	const size_t bricks_total = bricks_count.v[0] * bricks_count.v[1];
	const size_t x_count = bricks_count.v[0];
	const size_t y_count = bricks_count.v[1];

	// Create brick template
	noob::prop_blueprints brick_bp;
	brick_bp.mass = 5.0;
	brick_bp.shape = g.box_shape(brick_dims.v[0], brick_dims.v[1], brick_dims.v[2]);
	brick_bp.model = g.model_from_shape(brick_bp.shape, bricks_total);
	brick_bp.reflect = g.get_reflectance("default");
	brick_bp.ccd = false;

	// Create floor
	noob::scenery_handle floor = stage.create_scenery(g.box_shape(brick_dims.v[0] * x_count, 1.0, brick_dims.v[2]), noob::vec3f(brick_dims.v[0] * x_count * 0.5 - brick_dims.v[0] * 0.5, 0.0, brick_dims.v[2]), orient);

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
			noob::prop_handle brick_h = stage.create_prop(brick_bp_h, 0, noob::vec3f(static_cast<float>(xx) * brick_dims.v[0], static_cast<float>(yy) * brick_dims.v[1], brick_dims.v[2]), noob::versorf(0.0, 0.0, 0.0, 1.0));
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
   void create_cannon(const noob::vec3f & pos, const noob::versorf & orient, size_t bore_diameter, noob::stage & stage, noob::globals & g)
   {
   const float PI = 3.1415926;
   const float sector_step = 2.0 * PI / (static_cast<float>(bore_diameter) * 0.5);
   float sector_angle; 

   for (size_t i = 0; i <= bore_diameter / 2; ++i)
   {
   sector_angle = static_cast<float>(i) * sector_step;
   noob::scenery_handle sc_h = stage.create_scenery(
   g.cylinder_shape(0.5, 60.0, 12),
   noob::vec3f(
   pos.v[0] + static_cast<float>(bore_diameter) * 0.5 * std::cos(sector_angle),
   pos.v[1],
   pos.v[2] + static_cast<float>(bore_diameter) * 0.5 * std::sin(sector_angle)),
   orient);

   }

   stage.create_scenery(g.cylinder_shape(static_cast<float>(bore_diameter / 2), 60.0, 12), pos, orient);

// Cap the non-firing end of the cannon with a cylinder of appropriate width

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

	create_paddle_wheel(noob::vec3f(0.0, 0.0, 0.0), stage, g);


	noob::prop_blueprints ball_bp;
	ball_bp.mass = 10000.0;;
	ball_bp.shape = g.sphere_shape(0.5);
	ball_bp.model = g.model_from_shape(ball_bp.shape, 1);
	ball_bp.reflect = rh;
	ball_bp.ccd = true;

	noob::prop_blueprints_handle ball_bp_h = stage.add_props_blueprints(ball_bp);
	stage.reserve_props(ball_bp_h, 1);
	noob::prop_handle ball_h = stage.create_prop(ball_bp_h, 0, noob::vec3f(50.0, 100.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));


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
	const uint64_t profiling_interval = 3000;

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
