import ROOT
from ROOT import RooRealVar, RooHistPdf, RooDataHist, RooArgSet, TCanvas, RooFit, RooAbsPdf, TSpline3

# load C++ library
ROOT.gInterpreter.ProcessLine('#include "/cms/user/guojl/ME_test/CMSSW_10_6_26/src/HZZAnalysis/SignalModel/SplinePdf.h"')

#Analytical shape
# Define variables
def build_analytical_bw_shape(w,isplot = True):
    mass = ROOT.RooRealVar("mass", "Mass", 0, 4000)  # Assuming a mass range from 0 to 1000 GeV
    mean = ROOT.RooRealVar("mean", "Mean", 1000)  # Mean value with initial value and allowed range
    width = ROOT.RooRealVar("width", "Width", w)  # Width with initial value and allowed range

    # Define Breit-Wigner function
    #bw = ROOT.RooBreitWigner("bw", "Breit-Wigner", mass, mean, width)
    bw = ROOT.RooGenericPdf("bw",
                            "Breit-Wigner",
                            #"((mean*width)/pi)*((2*mass)/( ((mass*mass-mean*mean)*(mass*mass-mean*mean))+ ((mean*width)*(mean*width)) ))",
                            "((2*mass)/( ((mass*mass-mean*mean)*(mass*mass-mean*mean))+ ((mean*width)*(mean*width)) ))",
                           ROOT.RooArgSet(mass,mean,width))
    
    return bw

def build_dcb_from_exsis_file(mass):
    #retrieve Resolution from rootfile
    ggHshape = ROOT.TFile("Resolution/2l2q_resolution_merged_2018.root")
    #define current mass
    mH = 1000
    #build params
    ## tail parameters
    name = "a1_ggH_"
    a1_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("a1")).GetListOfFunctions().First().Eval(mH))
    name = "a2_ggH_"
    a2_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("a2")).GetListOfFunctions().First().Eval(mH))
    name = "n1_ggH_"
    n1_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("n1")).GetListOfFunctions().First().Eval(mH))
    name = "n2_ggH_"
    n2_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("n2")).GetListOfFunctions().First().Eval(mH))
    ##mean and sigma
    name = "mean_ggH_"
    mean_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("mean")).GetListOfFunctions().First().Eval(mH))
    mean_ggH.setVal(0)
    name = "sigma_ggH_"
    sigma_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("sigma")).GetListOfFunctions().First().Eval(mH))
    #sigma_ggH = ROOT.RooRealVar(name,name, 0)

    #build DCB
    name = "signalCB_ggH_"
    signalCB_ggH = ROOT.RooDoubleCB(name,name,mass,mean_ggH,sigma_ggH,a1_ggH,n1_ggH,a2_ggH,n2_ggH)
    #signalCB_ggH.setVal(0)

    return signalCB_ggH

def build_dcb_from_histogram(ifplot = True):
    hist_file = ROOT.TFile("resoliton_ggh1000.root")
    hist = hist_file.Get('ggh1000')

    # Define variables
    mass = ROOT.RooRealVar("mass", "Mass", -1000, 1000)

    # Create a RooDataHist from the histogram
    datahist = ROOT.RooDataHist("datahist", "Data Histogram", ROOT.RooArgList(mass), hist)

    # Define the parameters for the RooDoubleCB
    mean_dcb = ROOT.RooRealVar("mean_dcb", "Mean", 0, -1000, 1000)
    sigma_dcb = ROOT.RooRealVar("sigma_dcb", "Width", 0, 200)
    alpha1 = ROOT.RooRealVar("alpha1", "Alpha1", 0.1, 10)
    n1 = ROOT.RooRealVar("n1", "n1", 0.1,25)
    alpha2 = ROOT.RooRealVar("alpha2", "Alpha2", 0.1,10)
    n2 = ROOT.RooRealVar("n2", "n2", 0.1,25)

    # Create the RooDoubleCB PDF
    dcb = ROOT.RooDoubleCB("dcb", "Double Crystal Ball", mass, mean_dcb, sigma_dcb, alpha1, n1, alpha2, n2)

    # Fit the RooDoubleCB to the RooDataHist
    dcb.fitTo(datahist)

    if ifplot:
        # Create a plot
        frame = mass.frame()
        datahist.plotOn(frame)
        dcb.plotOn(frame)

        # Draw the plot
        canvas = ROOT.TCanvas("canvas", "Histogram Fit with RooDoubleCB", 800, 600)
        frame.Draw()
        canvas.SaveAs("dcb_fit_ggh100.png")

    return dcb

def build_splinepdf(m2l2q_gen,Mass,Width,production='ggf'):
    '''
    create a higgs pdf
    defualt production is ggf, if production is vbf, set production = 'vbf'
    input aguments m2l2q_gen, Mass and Width should be RooRealVar
    '''
    # input spline file
    f_spline = ROOT.TFile("/cms/user/guojl/ME_test/CMSSW_10_6_26/src/HZZAnalysis/SignalModel/GeliangShare/public/h4l_highmass/{}_input_spline.root".format(production))

    # load splines
    spline = f_spline.Get("sp_xsec_{}_2e2mu".format(production))

    # create SplinePdf
    higgspdf = ROOT.SplinePdf("higgspdf","higgspdf",m2l2q_gen,Mass,Width,spline)

    return higgspdf

#define this script as main
if __name__ == '__main__':

    #get dcb and bw
    #dcb = build_dcb_from_histogram(ifplot = False)
    #bw = build_analytical_bw_shape(647.0)
    low_mass = 0; high_mass = 3500; width_bw = 29.2; mH = 400
    #bw
    mass_zz2l2q = ROOT.RooRealVar("mass", "Mass", 0, 3500)  # Assuming a mass range from 0 to 1000 GeV
    mean = ROOT.RooRealVar("mean", "Mean", mH)  # Mean value with initial value and allowed range
    #width = ROOT.RooRealVar("width", "Width", 647.0)  # Width with initial value and allowed range #1000
    width = ROOT.RooRealVar("width", "Width", width_bw)  # Width with initial value and allowed range #500

    #build higgs pdf
    higgspdf = build_splinepdf(mass_zz2l2q,mean,width,production='ggf')

    #build sig histogram from higgs pdf
    nbins = 3500
    hsig_2l2q = ROOT.TH1F('hsig_2l2q','hsig_2l2q',nbins,low_mass,high_mass)
    for ib in range(1,nbins+1):
        x = hsig_2l2q.GetBinCenter(ib)
        mass_zz2l2q.setVal(x)
        mass_set = ROOT.RooArgSet(mass_zz2l2q)
        if(((x<mH-3*width_bw) & (x<mH-0.5*0.5)) | ((x>mH+3*width_bw) & (x>mH+0.5*0.5)) | (width_bw>10*0.5)):
            bc = higgspdf.getVal(mass_set)
            bc_range = higgspdf.getVal(mass_set)*0.5
        else:
            bc = 0
            bc_range = 0
            for j in range(-500,500):
                xx = mH + j*0.02*width_bw
                if(xx>x-0.5*0.5 & xx<=x+0.5*0.5):
                    mass_zz2l2q.setVal(xx)
                    mass_set = ROOT.RooArgSet(mass_zz2l2q)
                    bc += higgspdf.getVal(mass_set)
                    bc_range += higgspdf.getVal(mass_set)*0.02*width_bw
        if bc<0: bc = 0
        hsig_2l2q.SetBinContent(ib,bc)
    print("Integral of signal histogram: ", hsig_2l2q.Integral())

    # Create a RooDataHist from the histogram
    datahist = ROOT.RooDataHist("datahist", "Data Histogram", ROOT.RooArgList(mass_zz2l2q), hsig_2l2q)
    datapdf = ROOT.RooHistPdf("sigpdf", "sigpdf", ROOT.RooArgSet(mass_zz2l2q), datahist)
    
    # Define Breit-Wigner function
    #bw = ROOT.RooBreitWigner("bw", "Breit-Wigner", mass, mean, width)
    #bw = ROOT.RooGenericPdf("bw",
    #                        "Breit-Wigner",
    #                        #"((mean*width)/pi)*((2*mass)/( ((mass*mass-mean*mean)*(mass*mass-mean*mean))+ ((mean*width)*(mean*width)) ))",
    #                        "((2*mass)/( ((mass*mass-mean*mean)*(mass*mass-mean*mean))+ ((mean*width)*(mean*width)) ))",
    #                       ROOT.RooArgSet(mass_zz2l2q,mean,width))
    
    #dcb fit
    #hist_file = ROOT.TFile("resoliton_ggh1000.root")
    #hist = hist_file.Get('ggh1000')
#
    ## Define variables
    #mass = ROOT.RooRealVar("mass", "Mass", -500, 500)
#
    ## Create a RooDataHist from the histogram
    #datahist = ROOT.RooDataHist("datahist", "Data Histogram", ROOT.RooArgList(mass), hist)
#
    ## Define the parameters for the RooDoubleCB
    #mean_dcb = ROOT.RooRealVar("mean_dcb", "Mean", 0)
    #mean_dcb.setVal(0)
    #mean_dcb.setConstant(True)
    #sigma_dcb = ROOT.RooRealVar("sigma_dcb", "Width", 0, 200)
    #alpha1 = ROOT.RooRealVar("alpha1", "Alpha1", 0.1, 10)
    #n1_dcb = ROOT.RooRealVar("n1", "n1", 0.1,25)
    #alpha2 = ROOT.RooRealVar("alpha2", "Alpha2", 0.1,10)
    #n2_dcb = ROOT.RooRealVar("n2", "n2", 0.1,25)
#
    ## Create the RooDoubleCB PDF
    #dcb = ROOT.RooDoubleCB("dcb", "Double Crystal Ball", mass, mean_dcb, sigma_dcb, alpha1, n1_dcb, alpha2, n2_dcb)
#
    ## Fit the RooDoubleCB to the RooDataHist
    #dcb.fitTo(datahist)
#
    ##retrieve Parameter from fit result 
    #name = "a1_ggH_"
    #a1_ggH = ROOT.RooRealVar(name,name, alpha1.getVal())
    #name = "a2_ggH_"
    #a2_ggH = ROOT.RooRealVar(name,name, alpha2.getVal())
    #name = "n1_ggH_"
    #n1_ggH = ROOT.RooRealVar(name,name, n1_dcb.getVal())
    #name = "n2_ggH_"
    #n2_ggH = ROOT.RooRealVar(name,name, n2_dcb.getVal())
    ###mean and sigma
    #name = "mean_ggH_"
    ##mean_ggH = ROOT.RooRealVar(name,name, mean_dcb.getVal())
    #mean_ggH = ROOT.RooRealVar(name,name, 0)
    #mean_ggH.setVal(0)
    #mean_ggH.setConstant(True)
    #name = "sigma_ggH_"
    #sigma_ggH = ROOT.RooRealVar(name,name, sigma_dcb.getVal())
#
    ##build dcb
#
    #name = "signalCB_ggH_"
    #signalCB_ggH = ROOT.RooDoubleCB(name,name,mass_zz2l2q,mean_ggH,sigma_ggH,a1_ggH,n1_ggH,a2_ggH,n2_ggH)

    #retrieve Resolution from rootfile
    ggHshape = ROOT.TFile("2l2q_resolution_merged_2018.root")
    #define current mass
    #build params
    ## tail parameters
    name = "a1_ggH_"
    a1_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("a1")).GetListOfFunctions().First().Eval(mH))
    name = "a2_ggH_"
    a2_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("a2")).GetListOfFunctions().First().Eval(mH))
    name = "n1_ggH_"
    n1_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("n1")).GetListOfFunctions().First().Eval(mH))
    name = "n2_ggH_"
    n2_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("n2")).GetListOfFunctions().First().Eval(mH))
    ##mean and sigma
    name = "mean_ggH_"
    mean_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("mean")).GetListOfFunctions().First().Eval(mH))
    mean_ggH.setVal(0)
    name = "sigma_ggH_"
    sigma_ggH = ROOT.RooRealVar(name,name, (ggHshape.Get("sigma")).GetListOfFunctions().First().Eval(mH))
    #sigma_ggH = ROOT.RooRealVar(name,name, 0)

    #build DCB
    name = "signalCB_ggH_"
    signalCB_ggH = ROOT.RooDoubleCB(name,name,mass_zz2l2q,mean_ggH,sigma_ggH,a1_ggH,n1_ggH,a2_ggH,n2_ggH)


    #convlution PDF
    #signal_shape = ROOT.RooFFTConvPdf('convlution','convlution',mass_zz2l2q,bw,signalCB_ggH,2)
    #signal_shape = ROOT.RooFFTConvPdf('convlution','convlution',mass_zz2l2q,higgspdf,signalCB_ggH,2)
    signal_shape = ROOT.RooFFTConvPdf('convlution','convlution',mass_zz2l2q,datapdf,signalCB_ggH,2)
    print(signal_shape.getVal())  


    #get reco histo
    #hist_file = ROOT.TFile("histo_ggh{}.root".format(mH))
    #hist = hist_file.Get('ggh{}'.format(mH))
    #datahist = ROOT.RooDataHist("datahist", "Data Histogram", ROOT.RooArgList(mass_zz2l2q), hist)
    #datapdf = ROOT.RooHistPdf("pdf", "PDF from DataHist", ROOT.RooArgList(mass_zz2l2q), datahist)



    #mass = ROOT.RooRealVar("mass", "Mass", 0, 4000)
    ##Draw signal_shape#Draw
    canvas = ROOT.TCanvas("canvas", "BWxDCB Function Example")
    frame = mass_zz2l2q.frame()
    #datapdf.plotOn(frame, RooFit.DrawOption("F"),RooFit.FillColor(ROOT.kOrange), RooFit.LineColor(ROOT.kOrange),RooFit.FillStyle(1001),RooFit.MarkerStyle(1))
    #datapdf.plotOn(frame, RooFit.DrawOption("E"))
    signal_shape.plotOn(frame)
    frame.Draw()
    ##frame.SetMinimum(10e-8)
    ##frame.SetMaximum(10e-1)
    ##canvas.SetLogy()
    # Create a legend
    legend = ROOT.TLegend(0.6, 0.7, 0.85, 0.85)
    legend.AddEntry(signal_shape,"Model mH(1000) width = 647 (GeV)","1")
    #legend.AddEntry(datapdf,"ggH(1000) from MC","1")
    #legend.Draw()
    canvas.SetGrid()
    canvas.Draw()
    canvas.SaveAs("convolution_plot_{}.png".format(mH))