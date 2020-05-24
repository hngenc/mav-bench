import matplotlib.pyplot as plt
import sys
import numpy as np
import matplotlib.patches as patches
from mpl_toolkits.axes_grid1.inset_locator import zoomed_inset_axes
from mpl_toolkits.axes_grid1.inset_locator import mark_inset
from mpl_toolkits.mplot3d import Axes3D 
sys.path.append('../../common_utils')
from data_parsing import *


stage_of_interests_to_pick_from = ["pc_om", "om_to_pl", "pp_pl", "pc_om_estimation"]

# which stage are you trying to plot
stage_of_interest = "pc_om_estimation" # pick form ["om_to_pl", "pc_om", "pp_pl"]

assert stage_of_interest in stage_of_interests_to_pick_from

result_folder = stage_of_interest+"/data_1"
input_file_name = "stats.json"
input_filepath = result_folder + "/" + input_file_name

# data to collect
metrics_to_collect_easy = []
metrics_to_collect_hard = ["octomap_exposed_resolution", "point_cloud_estimated_volume", "octomap_volume_digested", "pc_vol_estimated",
        "potential_volume_to_explore_knob_modeling", "resolution_to_explore_knob_modeling", 
        "piecewise_planner_time_knob_modeling", "piecewise_planner_resolution_knob_modeling", "piecewise_planner_volume_explored_knob_modeling",
        "piecewise_planner_time_knob_modeling", "octomap_to_motion_planner_serialization_to_reception_knob_modeling", "octomap_insertCloud_minus_publish_all",
        "octomap_to_planner_com_overhead_knob_modeling", "pc_res", "pc_vol_actual", 
        "om_to_pl_res_knob_modeling", "om_to_pl_vol_actual_knob_modeling", "ppl_vol_actual_knob_modeling", ]

# parse  data
result_dic = parse_stat_file_flattened(input_filepath, metrics_to_collect_easy, metrics_to_collect_hard)
result_dic = filter_based_on_key_value(result_dic, "pc_res", 1.200000, "in")
#write_results_to_csv(result_dic, output_all_csv_filepath)
octomap_exposed_resolution = result_dic["octomap_exposed_resolution"]
point_cloud_estimated_volume  = result_dic["point_cloud_estimated_volume"]
#point_cloud_estimated_volume  = result_dic["octomap_volume_digested"]
#point_cloud_volume_to_digest = result_dic["point_cloud_volume_to_digest"]
#filtering = result_dic["filtering"]
octomap_volume_digested = result_dic["octomap_volume_digested"]
pc_vol_estimated = result_dic["pc_vol_estimated"]


octomap_integeration_response_time = result_dic["octomap_insertCloud_minus_publish_all"]
resolution_to_explore_knob_modeling = result_dic["resolution_to_explore_knob_modeling"]
piecewise_planner_resolution_knob_modeling = result_dic["piecewise_planner_resolution_knob_modeling"]
potential_volume_to_explore_knob_modeling= result_dic["potential_volume_to_explore_knob_modeling"]
piecewise_planner_time_knob_modeling = result_dic["piecewise_planner_time_knob_modeling"]
piecewise_planner_volume_explored_knob_modeling = result_dic["piecewise_planner_volume_explored_knob_modeling"]
octomap_to_motion_planner_serialization_to_reception_knob_modeling = result_dic["octomap_to_motion_planner_serialization_to_reception_knob_modeling"]
octomap_to_planner_com_overhead_knob_modeling = result_dic["octomap_to_planner_com_overhead_knob_modeling"]
pc_res = result_dic["pc_res"]
pc_vol_actual = result_dic["pc_vol_actual"]
om_to_pl_res = result_dic["om_to_pl_res_knob_modeling"]
om_to_pl_vol_actual = result_dic["om_to_pl_vol_actual_knob_modeling"]
ppl_vol_actual_knob_modeling = result_dic["ppl_vol_actual_knob_modeling"]


# -- filtering for debugging (probing into the data)
#filtered_results = filter_based_on_keys(result_dic, ["piecewise_planner_resolution_knob_modeling", "piecewise_planner_time_knob_modeling"])
#filtered_results = filter_based_on_key_value(filtered_results, "piecewise_planner_resolution_knob_modeling", 1.2, "in")

fig = plt.figure()
#if (stage_of_interest == "pc_om_estimation"):
#    ax = fig.add_subplot(111)
#else:
ax = fig.add_subplot(111, projection='3d')

# -- for point cloud/octomap (data_1/stats.json_om)
if stage_of_interest == "pc_om_estimation":
    print(len(pc_res))
    print(len(pc_vol_estimated))
    print(len(octomap_volume_digested))
    ax.scatter(pc_vol_estimated[1000:], octomap_volume_digested[1000:])#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
    #ax.scatter(pc_res[100:], pc_vol_estimated[100:], octomap_volume_digested[100:])#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
    print(pc_res)
    print(pc_vol_estimated)
    print(octomap_volume_digested)
elif stage_of_interest == "pc_om":
    ax.scatter(pc_res, pc_vol_actual, octomap_integeration_response_time)#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
elif stage_of_interest == "om_to_pl":
    ax.scatter(om_to_pl_res, om_to_pl_vol_actual, octomap_to_motion_planner_serialization_to_reception_knob_modeling)#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
    #ax.scatter(om_to_pl_res, potential_volume_to_explore_knob_modeling, octomap_to_planner_com_overhead_knob_modeling)#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
elif stage_of_interest == "pp_pl":
    ax.scatter(piecewise_planner_resolution_knob_modeling, ppl_vol_actual_knob_modeling, piecewise_planner_time_knob_modeling)#, zdir='z', c=None, depthshade=True)#(, *args, **kwargs)
else:
    print "stage of interest:" + stage_of_interest + "not defined" 
    system.exit(0)

# plot
if (stage_of_interest == "pc_om_estimation"):
    ax.set_xlabel('pc_vol_estimation')
    ax.set_ylabel('octomap_volume_digested')
    ax.set_ylabel('resolution')
else:
    ax.set_xlabel('resolution')
    ax.set_ylabel('estimated volume')
    ax.set_zlabel('response time (s)');
ax.legend(loc='best', fontsize="small")
output_file = "knob_performance_modeling" + ".png"
plt.show()
plt.savefig(result_folder+"/"+output_file)
plt.close(fig)
