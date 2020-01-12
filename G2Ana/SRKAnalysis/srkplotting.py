import srkdata
import numpy as np
import ROOT as r
import matplotlib.pyplot as plt
#plt.style.use('mastercolor')
import srkglobal
import srkmisc

default_params = ["posX", "posY", "posZ", "velX", "velY", "velZ", "MX", "MY", "MZ", "magM", "projection"]#, "Bx", "By", "Bz"]
B_list = ["Bx", "By", "Bz", "magB"]
E_list = ["Ex", "Ey", "Ez", "magE"]

def make_tree_to_array(run_id, query_list=default_params):
	"""Makes the step_tree of result files into a numpy array"""
	#Currently only for one particle!
	
#	print query_list
	results_path = srkglobal.results_dir + "Results_RID" + str(run_id) + ".root"
	if not srkmisc.file_exits_and_not_zombie(results_path):
		print "No results file found for RunID " + str(run_id) + ". Aborting."
		return
	else:
		root_file = r.TFile(results_path, "READ")
		hit_tree = r.gDirectory.Get('hitTree')
#		r.gROOT.cd()
		
		return_arr = np.empty((len(query_list), hit_tree.GetEntries()))
#		tree_dict = {
#			"posX":step_tree.pos.x(),
#			"posZ":step_tree.pos.z(),
#			"velX":step_tree.vel.x(),
#			"velZ":step_tree.vel.z(),
#			"phi":step_tree.phi,
#			"theta":step_tree.theta()
#		}
		for i in range(hit_tree.GetEntries()):
			hit_tree.GetEntry(i)
			return_arr[0,i] = hit_tree.pos.x()
			return_arr[1,i] = hit_tree.pos.y()
			return_arr[2,i] = hit_tree.pos.z()
			return_arr[3,i] = hit_tree.vel.x()
			return_arr[4,i] = hit_tree.vel.y()
			return_arr[5,i] = hit_tree.vel.z()
			return_arr[6,i] = hit_tree.M.x()
			return_arr[7,i] = hit_tree.M.y()
			return_arr[8,i] = hit_tree.M.z()
			return_arr[9,i] = hit_tree.magM
			return_arr[10,i] = hit_tree.projection

		try:
			B_arr = np.empty((len(B_list), hit_tree.GetEntries()))
			for j in range(hit_tree.GetEntries()):
				B_arr[0,j] = hit_tree.BField.x()
				B_arr[1,j] = hit_tree.BField.y()
				B_arr[2,j] = hit_tree.BField.z()
				B_arr[3,j] = hit_tree.BFieldStrength
		except:
			print "B field data was not recorded"
		else:
			return_arr = np.vstack((return_arr, B_arr))
			
		try:
			E_arr = np.empty((len(E_list), hit_tree.GetEntries()))
			for j in range(hit_tree.GetEntries()):
				E_arr[0,j] = hit_tree.EField.x()
				E_arr[1,j] = hit_tree.EField.y()
				E_arr[2,j] = hit_tree.EField.z()
				E_arr[3,j] = hit_tree.EFieldStrength
		except:
			print "E field data was not recorded"
		else:
			return_arr = np.vstack((return_arr, E_arr))
#			for j in range(len(query_list)):
#				return_arr[j,i]=tree_dict[query_list[j]]
		
		root_file.Close()
		return return_arr
		
def make_timeline(run_id):
	srk_settings, run_settings = srkdata.get_settings_from_database(run_id, "g2")
	no = srk_settings['TimeLimit']/srk_settings['PeriodicStopTime'] + 1
	time_arr = np.linspace(0, srk_settings['TimeLimit'], no)
	#instead use time from root file?
	return time_arr
	
def delta_phi(arr, baseline, absolute=False):
	if len(arr) != len(baseline):
		print "Arrays not of same length, aborting!" #later maybe length of shortest? 
		return
	new_arr = np.empty(len(arr))
	if absolute: 
		new_arr = arr-baseline
	else:
		new_arr[0] = np.nan
		new_arr[1:] = (arr[1:]-baseline[1:])/(baseline[1:]-baseline[0])
	return new_arr
	
def plot_single(title, arr, timeline, savemode = 'both', titleY = False, titleX = 't [s]', legend = False, style=False, rangeX=False, rangeY=False):
	if not legend: legend = ''
	if not style: style='-'
	plt.plot(timeline, arr, style, label=legend)
	plt.legend(loc='best')
	plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
	plt.xlabel(titleX)
	plt.title(title)
	plt.tick_params(right=True)
	if titleY: plt.ylabel(titleY)
	if rangeX: plt.xlim((rangeX[0], rangeX[1]))
	if rangeY: plt.ylim((rangeY[0], rangeY[1]))
	if not savemode in ['plot', 'show']: 
		nameparts = title.split()
		savetitle = ''
		for n in nameparts:
			savetitle += n + '_'
		savetitle = savetitle[:-1]
		plt.savefig(srkglobal.graphs_dir+savetitle+'.png') #save to srkglobal graphs directory?
	if savemode != 'save': plt.show()
	
def plot_comparative(title, arr_list, timelines, savemode = 'both', titleY = False, titleX = 't [s]', legend_list = False, style=False, rangeX=False, rangeY=False):
	if not legend_list: legend_list = [i for i in range(len(arr_list))]
	if not style: style = '-' #later also list of styles? careful, string = list of chars
	if np.linalg.matrix_rank(timelines) == 1:
		timelines = [timelines for i in range(len(arr_list))]
	for i in range(len(arr_list)):
		plt.plot(timelines[i], arr_list[i], style, label=legend_list[i])
	plt.ticklabel_format(style='sci', axis='x', scilimits=(0,0))
	plt.legend(loc='best')
	plt.xlabel(titleX)
	plt.title(title)
	plt.tick_params(right=True)
	if titleY: plt.ylabel(titleY)
	if rangeX: plt.xlim((rangeX[0], rangeX[1]))
	if rangeY: plt.ylim((rangeY[0], rangeY[1]))
	if not savemode in ['plot', 'show']:
		nameparts = title.split()
		savetitle = ''
		for n in nameparts:
			savetitle += n + '_'
		savetitle = savetitle[:-1]
		plt.savefig(srkglobal.graphs_dir+savetitle+'.png')
	if savemode != 'save': plt.show()
