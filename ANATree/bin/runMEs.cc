#include <iostream>
//Auto MELA
#include "IvyBase.h"
#include <IvyFramework/IvyAutoMELA/interface/IvyMELAHelpers.h>
#include <IvyFramework/IvyAutoMELA/interface/IvyMELAOutputStreamerExt.h>
#include <IvyFramework/IvyDataTools/interface/HostHelpersCore.h>
#include "TString.h"
#include "TreeLoop.h"
#include "Settings.h"

void print_help(){
    printf("    --help, -h     :   print help\n");
    printf("    --tree, -t     :   tree style output\n");
}


void read_options(int argc, char *argv[], Options &opt){
  if(argc < 1){
    print_help();
    exit(1);
  }

  for(int i = 1; i<argc; ++i){
    if(argv[i][0] == '-'){
      if(strcmp("--tree",argv[i]) == 0 || strcmp("-t",argv[i]) == 0){
        opt.tree = true;
      }else if(strcmp("--inputpath",argv[i]) == 0 || strcmp("-p",argv[i]) == 0){
        opt.input_file_path = argv[i+1];
        opt.output_file_path = opt.input_file_path+"/skimed/";
      }else if(strcmp("--inputfile",argv[i]) == 0 || strcmp("-i",argv[i]) == 0){
        opt.infile = argv[i+1];
        opt.infile = opt.input_file_path+'/'+opt.infile;
        opt.outfile = opt.output_file_path+argv[i+1];
      }
    }
  }
}

int main(int argc, char *argv[]){
  using namespace std;
  using namespace IvyStreamHelpers;

  Options opt;
  read_options(argc, argv,opt);
  cout<<"input file ="<<opt.infile<<endl;
  cout<<"out file ="<<opt.outfile<<endl;
  TreeLoop *lp = new TreeLoop(opt.infile,opt.outfile);
  lp->Loop();
  //TreeLoop loop("${CMSSW_BASE}/src/HZZAnalysis/ANATree/testfile/test.root");


  cout<<"haha"<<endl;
  return 0;
}
