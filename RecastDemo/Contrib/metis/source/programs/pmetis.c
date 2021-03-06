/*
 * Copyright 1997, Regents of the University of Minnesota
 *
 * pmetis.c
 *
 * This file contains the driving routine for multilevel method
 *
 * Started 8/28/94
 * George
 *
 * $Id: pmetis.c,v 1.3 2002/08/10 06:57:51 karypis Exp $
 *
 */

#include <metisbin.h>



/*************************************************************************
* Let the game begin
**************************************************************************/
int main(int argc, char *argv[])
{
  idxtype i, options[10];
  idxtype *part;
  float lbvec[MAXNCON];
  GraphType graph;
  idxtype numflag = 0, wgtflag = 0, edgecut;
  ParamType params;
  double TOTALTmr, METISTmr, IOTmr;


  parse_cmdline(&params, argc, argv);


  if (params.nparts < 2) {
    mprintf("The number of partitions should be greater than 1!\n");
    exit(0);
  }

  gk_clearcputimer(TOTALTmr);
  gk_clearcputimer(METISTmr);
  gk_clearcputimer(IOTmr);

  gk_startcputimer(TOTALTmr);
  gk_startcputimer(IOTmr);
  ReadGraph(&graph, params.filename, &wgtflag);
  if (graph.nvtxs <= 0) {
    mprintf("Empty graph. Nothing to do.\n");
    exit(0);
  }
  gk_stopcputimer(IOTmr);

  mprintf("**********************************************************************\n");
  mprintf("%s", METISTITLE);
  mprintf("Graph Information ---------------------------------------------------\n");
  mprintf("  Name: %s, #Vertices: %D, #Edges: %D, #Parts: %D\n", params.filename, graph.nvtxs, graph.nedges/2, params.nparts);
  if (graph.ncon > 1)
    mprintf("  Balancing Constraints: %D\n", graph.ncon);
  mprintf("\nRecursive Partitioning... -------------------------------------------\n");

  part = idxmalloc(graph.nvtxs, "main: part");
  options[0] = 0;
  options[0] = 1;
  options[OPTION_CTYPE]  = params.mtype;
  options[OPTION_ITYPE]  = params.itype;
  options[OPTION_RTYPE]  = params.rtype;
  options[OPTION_DBGLVL] = params.dbglvl;

  gk_startcputimer(METISTmr);
  if (graph.ncon == 1) {
    METIS_PartGraphRecursive(&graph.nvtxs, graph.xadj, graph.adjncy, graph.vwgt, graph.adjwgt, 
          &wgtflag, &numflag, &(params.nparts), options, &edgecut, part);
  }
  else {
    METIS_mCPartGraphRecursive(&graph.nvtxs, &graph.ncon, graph.xadj, graph.adjncy, graph.vwgt, 
          graph.adjwgt, &wgtflag, &numflag, &(params.nparts), options, &edgecut, part);
  }

  gk_stopcputimer(METISTmr);

  ComputePartitionBalance(&graph, params.nparts, part, lbvec);

  mprintf("  %D-way Edge-Cut: %7D, Balance: ", params.nparts, edgecut);
  for (i=0; i<graph.ncon; i++)
    mprintf("%5.2f ", lbvec[i]);
  mprintf("\n");


  gk_startcputimer(IOTmr);
  WritePartition(params.filename, part, graph.nvtxs, params.nparts); 
  gk_stopcputimer(IOTmr);
  gk_stopcputimer(TOTALTmr);

  mprintf("\nTiming Information --------------------------------------------------\n");
  mprintf("  I/O:          \t\t %7.3f\n", gk_getcputimer(IOTmr));
  mprintf("  Partitioning: \t\t %7.3f   (PMETIS time)\n", gk_getcputimer(METISTmr));
  mprintf("  Total:        \t\t %7.3f\n", gk_getcputimer(TOTALTmr));
  mprintf("**********************************************************************\n");


  gk_free((void **)&graph.xadj, &graph.adjncy, &graph.vwgt, &graph.adjwgt, &part, LTERM);
}  


