#!/usr/bin/python3

import numpy as np 
#from ROOT import TFile, gDirectory, gROOT 
import ROOT as r
import sys

filenames = sys.argv[1:]
for f in filenames:
	try:
		root_file = r.TFile(f, "READ")
	except:
		print(f + " could not be resolved. Skipping.")
		root_file = None

	if root_file:
		step_tree = r.gDirectory.Get('stepTree')
		r.gROOT.cd()

		x_array = np.empty(step_tree.GetEntries())
		vx_array = np.empty(step_tree.GetEntries())
		z_array = np.empty(step_tree.GetEntries())
		vz_array = np.empty(step_tree.GetEntries())
		phi_array = np.empty(step_tree.GetEntries())
		theta_array = np.empty(step_tree.GetEntries())

		for i in range(step_tree.GetEntries()):
			step_tree.GetEntry(i)
			x_array[i] = step_tree.pos.x()
			z_array[i] = step_tree.pos.z()
			vx_array[i] = step_tree.vel.x()
			vz_array[i] = step_tree.vel.z()
			phi_array[i] = step_tree.phi
			theta_array[i] = step_tree.theta

		root_file.Close()

	n = f.split(".")[0]
	np.savetxt(n + "_x.txt", x_array)
	np.savetxt(n + "_z.txt", z_array)
	np.savetxt(n + "_vx.txt", vx_array)
	np.savetxt(n + "_vz.txt", vz_array)
	np.savetxt(n + "_phi.txt", phi_array)
	np.savetxt(n + "_theta.txt", theta_array)
