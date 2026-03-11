-- Copyright 2016 The Cartographer Authors
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- you may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--      http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

include "map_builder.lua"
include "trajectory_builder.lua"

options = {
  map_builder = MAP_BUILDER,
  trajectory_builder = TRAJECTORY_BUILDER,
  map_frame = "map",
  tracking_frame = "robot/base_link/base_imu",
  published_frame = "base_link",
  odom_frame = "odom",

  -- We are 'using' odometry, so we don't 'provide' it.
  provide_odom_frame = false,
  publish_frame_projected_to_2d = true,

  -- Sensor configuration
  use_odometry = false,
  use_nav_sat = false,
  use_landmarks = false,
  num_laser_scans = 1,
  num_multi_echo_laser_scans = 0,
  num_subdivisions_per_laser_scan = 1,
  num_point_clouds = 0,

  -- Other settings
  publish_tracked_pose = true,
  lookup_transform_timeout_sec = 0.2,
  submap_publish_period_sec = 0.3,
  pose_publish_period_sec = 0.01,
  trajectory_publish_period_sec = 0.03,
  rangefinder_sampling_ratio = 1.0,
  odometry_sampling_ratio = 1.0,
  fixed_frame_pose_sampling_ratio = 1.0,
  imu_sampling_ratio = 1.0,
  landmarks_sampling_ratio = 1.0,
  use_pose_extrapolator = true,
}

MAP_BUILDER.use_trajectory_builder_3d = false
MAP_BUILDER.use_trajectory_builder_2d = true
MAP_BUILDER.num_background_threads = 4

-- This block defines local SLAM. It remains identical to your mapping file.
TRAJECTORY_BUILDER_2D = {
  min_range = 0.1,
  max_range = 30.0,
  num_accumulated_range_data = 1,
  voxel_filter_size = 0.025,
  use_imu_data = true,
  imu_gravity_time_constant = 10.0,

  adaptive_voxel_filter_options = {
    max_length = 0.5,
    min_num_points = 200,
    max_range = 30.0,
  },

  use_online_correlative_scan_matching = true,
  real_time_correlative_scan_matcher = {
    linear_search_window = 0.1,
    angular_search_window = math.rad(10.),
    translation_delta_cost_weight = 1e-1,
    rotation_delta_cost_weight = 1e-1,
  },

  ceres_scan_matcher = {
    translation_weight = 10.0,
    rotation_weight = 40.0,
    ceres_solver_options = {
      use_nonmonotonic_steps = false,
      max_num_iterations = 20,
      num_threads = 1,
    },
  },

  submaps = {
    num_range_data = 90,
    resolution = 0.05,
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

-- ########################################################################################
-- ##                  MOST IMPORTANT CHANGES ARE IN THE POSE_GRAPH                      ##
-- ########################################################################################

POSE_GRAPH = {
  --- CHANGE: This is the MASTER switch for localization mode.
  -- Setting to 0 disables the pose graph optimizer, effectively freezing the map.
  optimize_every_n_nodes = 0,

  --- CHANGE: Set to a low value to force continuous global searching.
  -- This makes Cartographer constantly try to find its place in the map.
  global_constraint_search_after_n_seconds = 1.0,

  max_num_final_iterations = 200,
  global_sampling_ratio = 0.003,
  log_residual_histograms = false,

  constraint_builder = {
    sampling_ratio = 0.3,
    max_constraint_distance = 15.0,
    
    --- CHANGE: Require a higher score for confident localization.
    min_score = 0.55,
    global_localization_min_score = 0.6,
    
    loop_closure_translation_weight = 1.1e4,
    loop_closure_rotation_weight = 1e5,

    fast_correlative_scan_matcher_2d = {
      linear_search_window = 7.0,
      angular_search_window = math.rad(30.0),
      branch_and_bound_depth = 7,
    },

    ceres_scan_matcher_2d = {
      translation_weight = 20.0,
      rotation_weight = 40.0,
      ceres_solver_options = {
        use_nonmonotonic_steps = false,
        max_num_iterations = 10,
        num_threads = 1,
      },
    },
  },

  -- This problem is still used to integrate odometry and IMU data
  -- to place the robot in the frozen map.
  optimization_problem = {
    huber_scale = 5e1,
    odometry_translation_weight = 1e5,
    odometry_rotation_weight = 1e5,
    acceleration_weight = 1e1,
    rotation_weight = 1e2,
    log_solver_summary = true,

    ceres_solver_options = {
      use_nonmonotonic_steps = false,
      max_num_iterations = 100,
      num_threads = 4,
    },
  },
}

--- CHANGE: This is new and important for localization.
-- It prevents memory from growing by only keeping a few
-- of the most recent "pure localization" submaps.
TRAJECTORY_BUILDER.pure_localization_trimmer = {
  max_submaps_to_keep = 3,
}

return options