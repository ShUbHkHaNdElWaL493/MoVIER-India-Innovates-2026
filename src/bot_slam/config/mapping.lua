include "map_builder.lua"
include "trajectory_builder.lua"

options = {
  map_builder = MAP_BUILDER,
  trajectory_builder = TRAJECTORY_BUILDER,
  
  -- 1. FRAME SETUP
  map_frame = "map",
  tracking_frame = "imu_link",       -- Kept as imu_link to prevent the colocation crash
  
  -- CRITICAL FIX: Cartographer connects the map to the EKF's odometry
  published_frame = "odom",          
  
  odom_frame = "odom",
  provide_odom_frame = false,        -- EKF provides odom -> base_link
  publish_frame_projected_to_2d = true,

  -- 2. SENSOR INPUT LOGIC
  use_odometry = true,               -- Listen to the EKF!
  use_nav_sat = false,
  use_landmarks = false,
  publish_tracked_pose = true,

  -- 3. LIDAR SETUP
  num_laser_scans = 1,
  num_multi_echo_laser_scans = 0,
  num_subdivisions_per_laser_scan = 1,
  num_point_clouds = 0,

  -- TIMING
  lookup_transform_timeout_sec = 0.2,
  submap_publish_period_sec = 0.3,
  pose_publish_period_sec = 5e-3,
  trajectory_publish_period_sec = 30e-3,
  rangefinder_sampling_ratio = 1.0,
  odometry_sampling_ratio = 1.0,
  fixed_frame_pose_sampling_ratio = 1.0,
  imu_sampling_ratio = 1.0,
  landmarks_sampling_ratio = 1.0,
}

MAP_BUILDER.use_trajectory_builder_3d = false
MAP_BUILDER.use_trajectory_builder_2d = true

-- Jetson Xavier multi-threading
MAP_BUILDER.num_background_threads = 4 

-- ==========================================
-- LOCAL SLAM (Building the Submaps)
-- ==========================================
TRAJECTORY_BUILDER_2D = {
  use_imu_data = false,              -- EKF already handles the IMU perfectly
  min_range = 0.15,
  max_range = 8.0,
  min_z = -40.0,
  max_z = 40.0,
  missing_data_ray_length = 8.5,
  num_accumulated_range_data = 1,
  voxel_filter_size = 0.025,

  adaptive_voxel_filter_options = {
    max_length = 0.5,                -- Reverted to 0.5 to prevent dropping too much pointcloud data
    min_num_points = 200,
    max_range = 8.0,
  },

  -- THE FIX: Handcuff Cartographer to the EKF for local movements
  use_online_correlative_scan_matching = true,
  real_time_correlative_scan_matcher = {
    linear_search_window = 0.1,      -- Strict 10cm limit! No more meter-scale jumps.
    angular_search_window = math.rad(10.0), 
    translation_delta_cost_weight = 10.0, 
    rotation_delta_cost_weight = 10.0,    
  },

  ceres_scan_matcher = {
    translation_weight = 100.0,      -- High weight to trust EKF translation
    rotation_weight = 100.0,         -- High weight to trust EKF rotation
    ceres_solver_options = {
      use_nonmonotonic_steps = true,
      max_num_iterations = 40,
      num_threads = 2,
    },
  },

  submaps = {
    num_range_data = 90,             -- Dense submaps to prevent blurry scans during turns
    grid_options_2d = {
      grid_type = "PROBABILITY_GRID",
      resolution = 0.05,
    },
    range_data_inserter = {
      range_data_inserter_type = "ProbabilityGridRangeDataInserter2D",
      probability_grid_range_data_inserter = {
        hit_probability = 0.55,
        miss_probability = 0.49,
      },
    },
  },
  
  motion_filter = {
    max_time_seconds = 5.0,
    max_distance_meters = 0.2,
    max_angle_radians = math.rad(1.0),
  },
}

-- ==========================================
-- GLOBAL SLAM (Loop Closure & Optimization)
-- ==========================================
POSE_GRAPH = {
  optimize_every_n_nodes = 15,
  global_sampling_ratio = 0.1,
  max_num_final_iterations = 100,
  log_residual_histograms = true,
  global_constraint_search_after_n_seconds = 10.0,

  constraint_builder = {
    sampling_ratio = 0.6,            
    max_constraint_distance = 8.0,
    min_score = 0.81,                -- Keeping your strict loop closure score
    global_localization_min_score = 0.65,
    loop_closure_translation_weight = 1.1e4,
    loop_closure_rotation_weight = 1e5,

    fast_correlative_scan_matcher_2d = {
      linear_search_window = 3.0,
      angular_search_window = math.rad(10.0),
      branch_and_bound_depth = 7,
    },

    ceres_scan_matcher_2d = {
      translation_weight = 10.0,
      rotation_weight = 1.0,
      ceres_solver_options = {
        use_nonmonotonic_steps = false,
        max_num_iterations = 40,
        num_threads = 2,
      },
    },
  },

  optimization_problem = {
    huber_scale = 1e1,
    
    -- Tell the optimizer to highly trust the EKF odometry
    odometry_translation_weight = 1e5,
    odometry_rotation_weight = 1e5,
    
    -- Zero out IMU weights since Cartographer isn't processing raw IMU anymore
    acceleration_weight = 0.0,
    rotation_weight = 0.0,

    log_solver_summary = false,
    ceres_solver_options = {
      use_nonmonotonic_steps = false,
      max_num_iterations = 50,
      num_threads = 4,
    },
  },
}

return options
