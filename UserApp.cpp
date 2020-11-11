#include "Application.hpp"
#include "Font.hpp"

std::unique_ptr<std::string> message_profiling, message_collision;

noob::actor_handle ah;
noob::prop_handle ph;
noob::font example_font;

std::string test_message;

noob::vec3f random_vec3(float x, float y, float z)
{
	return noob::vec3f(noob::random::get() * x, noob::random::get() * y, noob::random::get() * z);
}


noob::versorf random_versor()
{
	return noob::normalize(noob::versorf(noob::random::get(), noob::random::get(), noob::random::get(), noob::random::get()));
}


bool noob::application::user_init()
{
	message_profiling = std::make_unique<std::string>("");
	message_collision = std::make_unique<std::string>("");
	noob::logger::log(noob::importance::INFO, "[Application] Begin user init!");

	noob::globals& g = noob::get_globals();

	const noob::shape_handle scenery_shp = g.box_shape(100.0, 1.0, 100.0);//sphere_shape(50.0); 
	const noob::scenery_handle sc_h = stage.create_scenery(scenery_shp, noob::vec3f(0.0, 0.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));

	noob::reflectance r;
	r.set_specular(noob::vec3f(0.1, 0.1, 0.1));
	r.set_diffuse(noob::vec3f(0.1, 0.1, 0.1));
	r.set_emissive(noob::vec3f(0.0, 0.0, 0.0));
	r.set_shine(0.8);
	r.set_albedo(0.3);
	r.set_fresnel(0.2);

	const noob::reflectance_handle rh = g.reflectances.add(r);

	const float actor_dims = 1.0;
	// const noob::shape_handle shp = g.sphere_shape(actor_dims);
	const noob::shape_handle unit_cube_shp = g.box_shape(1.0, 1.0, 1.0); // sphere_shape(actor_dims);
	const noob::shape_handle unit_sphere_shp = g.sphere_shape(0.5);
	const uint32_t actor_count = 50;
/*
	noob::actor_blueprints bp;
	bp.bounds = unit_cube;
	bp.reflect = rh;
	bp.model = g.model_from_shape(unit_cube, actor_count*8);

	noob::actor_blueprints_handle abph = stage.add_actor_blueprints(bp);

	stage.reserve_actors(abph, actor_count * 8);

	for (uint32_t i = 0; i < actor_count; ++i)
	{	
		ah = stage.create_actor(abph, 0, random_vec3(stage_dim, stage_dim, stage_dim), random_versor());
	}

	logger::log(noob::importance::INFO, "Done making actors");
*/

	noob::scenery_handle sc2_h = stage.create_scenery(unit_cube_shp, noob::vec3f(0.0, 50.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));

	const size_t prop_count = 30;
	const float stage_dim = static_cast<float>(prop_count);
	noob::prop_blueprints pb;
	pb.mass = 5.0;
	pb.model =  g.model_from_shape(unit_sphere_shp, prop_count*8);
	pb.shape = unit_cube_shp;
	pb.reflect = rh;
	pb.ccd = false;
	
	noob::prop_blueprints_handle pbph = stage.add_props_blueprints(pb);

	stage.reserve_props(pbph, prop_count * 8);
	float y_accum = 38.0;
	noob::body_handle last_bod = stage.get_scenery(sc2_h).body;
	for (uint32_t i = 0; i < prop_count; ++i)
	{
		ph = stage.create_prop(pbph, 0, noob::vec3f(0.0, y_accum, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
		noob::body_handle current_bod = stage.get_prop(ph).body;
		noob::constraint_handle pc = stage.create_point_constraint(current_bod, last_bod, noob::vec3f(0.0, 0.5, 0.0), noob::vec3f(0.0, 0.0, 0.0)); 
		
		y_accum -= 1.0;
		last_bod = current_bod;
	}

	noob::shape_handle large_sphere_shp = g.sphere_shape(7.0);
	noob::prop_blueprints wrecking_ball_bp;
	wrecking_ball_bp.mass = 1000.0;
	wrecking_ball_bp.shape = large_sphere_shp;
	wrecking_ball_bp.model = g.model_from_shape(large_sphere_shp, 10);
	wrecking_ball_bp.reflect = rh;
	wrecking_ball_bp.ccd= true;

	auto wrecking_ball_bp_h = stage.add_props_blueprints(wrecking_ball_bp);
	stage.reserve_props(wrecking_ball_bp_h, 10);
	auto wrecking_ball_h = stage.create_prop(wrecking_ball_bp_h, 0, noob::vec3f(0.0, y_accum + 15.0, 0.0), noob::versorf(0.0, 0.0, 0.0, 1.0));
	
	auto wrecking_ball_bod = stage.get_prop(wrecking_ball_h).body;
	auto constrain_h = stage.create_point_constraint(wrecking_ball_bod, last_bod, noob::vec3f(0.0, 7.0, 0.0), noob::vec3f(0.0, 0.5, 0.0));

	// const noob::vec2d test_val = noob::vec2d(1.0, 0.7);
	// uint32_t db_idx = db.vec2fp_add(test_val);
	// noob::vec2d ret_val = db.vec2fp_get(db_idx).value;
	// noob::logger::log(noob::importance::INFO, noob::concat("[Application] Inserted value: ", noob::to_string(test_val), ". Returned value: ", noob::to_string(ret_val), "."));

	logger::log(noob::importance::INFO, "[Application] Successfully done (C++) user init.");
	paused = false;

	return true;
}


void noob::application::user_update(double dt)
{
	app_gui.text("Happy happy fun fun!!!!", noob::vec2f(100.0, 100.0), noob::gui::font_size::READING, noob::vec4f(1.0, 1.0, 0.0, 1.0));
	
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
