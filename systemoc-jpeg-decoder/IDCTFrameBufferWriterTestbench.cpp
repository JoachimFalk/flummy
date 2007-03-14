//  -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 expandtab:
/*
 * Copyright (c) 2007 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#include <list>
#include <set>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#include "FileSource.hpp"
// Begin IDCT2D
#include "block2row.hpp"
#include "IDCT2d.hpp"
#include "col2block.hpp"
// End IDCT2D
#include "InvLevel.hpp"
#include "Clip.hpp"
#include "FrameBufferWriter.hpp"

struct Scan {
  IntCompID_t scanPattern[SCANPATTERN_LENGTH];
  std::string idctCoeffFileName;
};

typedef std::list<Scan> ScanVector;

class IDCTScanSource: public smoc_actor {
public:
  smoc_port_out<IDCTCoeff_t>    out;
  smoc_port_out<JpegChannel_t>  outCtrlImage;
protected:
  size_t      width, height;
  ScanVector  scanVector;

  std::ifstream inputStream;

  void sendNewFrame() {
    std::set<IntCompID_t> componentSet;
    
    for (ScanVector::const_iterator iter = scanVector.begin();
         iter != scanVector.end();
         ++iter) {
      const Scan &scan = *iter;
      
      for (size_t i = 0; i < SCANPATTERN_LENGTH; ++i) {
        componentSet.insert(scan.scanPattern[i]);
      }
    }
    std::cerr << "IDCTScanSource: sendNewFrame"
      << " witdh: " << width
      << " height: " << height
      << " component count: " << componentSet.size()
      << std::endl;
    outCtrlImage[0] = JS_CTRL_NEWFRAME_SET_CHWORD
      (width, height, componentSet.size());
  }

  void sendNewScan() {
    const Scan &scan = scanVector.front();
    
    std::cerr << "IDCTScanSource: sendNewScan"
      << " from file: " << scan.idctCoeffFileName
      << " scanPattern: ";
    for (size_t i = 0; i < SCANPATTERN_LENGTH; ++i) {
      std::cerr
        << static_cast<unsigned int>(scan.scanPattern[i]);
//      << (i < SCANPATTERN_LENGTH-1 ? ":" : "");
    }
    std::cerr << std::endl;
    outCtrlImage[0] = JS_CTRL_NEWSCAN_SET_CHWORD
      (scan.scanPattern[0], scan.scanPattern[1], scan.scanPattern[2],
       scan.scanPattern[3], scan.scanPattern[4], scan.scanPattern[5]);
    inputStream.open(scan.idctCoeffFileName.c_str());
    assert(inputStream.good());
    scanVector.pop_front();
  }

  void sendIDCTCoeff() {
    IDCTCoeff_t coeff;
    
    inputStream >> coeff;
//  std::cerr << "IDCTScanSource: Got IDCT Coeff " << coeff << std::endl;
    out[0] = coeff;
  }

  void closeStream() {
    inputStream.close();
    // Reset EOF and ERROR bits
    inputStream.clear();
  }

  void allDone() {
    std::cerr << "IDCTScanSource: All done !!!" << std::endl;
  }

  bool haveScans() const
    { return !scanVector.empty(); }

  bool streamValid() const
    { return inputStream.good(); }

  smoc_firing_state start;
  smoc_firing_state scanNew;
  smoc_firing_state scanSend;
  smoc_firing_state end;
public:
  IDCTScanSource(sc_module_name name,
      size_t width, size_t height, const ScanVector &scanVector)
    : smoc_actor(name, start),
      width(width), height(height), scanVector(scanVector) {
    start
      = outCtrlImage(1)                       >>
        CALL(IDCTScanSource::sendNewFrame)    >> scanNew
      ;
    scanNew
      = GUARD(IDCTScanSource::haveScans)      >>
        outCtrlImage(1)                       >>
        CALL(IDCTScanSource::sendNewScan)     >> scanSend
      | !GUARD(IDCTScanSource::haveScans)     >>
        CALL(IDCTScanSource::allDone)         >> end
      ;
    scanSend
      = GUARD(IDCTScanSource::streamValid)    >>
        out(1)                                >>
        CALL(IDCTScanSource::sendIDCTCoeff)   >> scanSend
      | !GUARD(IDCTScanSource::streamValid)   >>
        CALL(IDCTScanSource::closeStream)     >> scanNew
      ;
  }
};

class Testbench: public smoc_graph {
private:
  IDCTScanSource    mIDCTScanSource;
  // Begin IDCT2D
  m_block2row       mBlock2Row;
  m_idct2d          mIDCT2D;
  m_col2block       mCol2Block;
  // End IDCT2D
//Round             mRound;
  InvLevel          mInvLevel;
  Clip              mClip;
  FrameBufferWriter mSink;
public:
  Testbench(sc_module_name name, size_t width, size_t height, const ScanVector &scanVector)
    : smoc_graph(name),
      mIDCTScanSource("mIDCTScanSource", width, height, scanVector),
      // Begin IDCT2D
      mBlock2Row("mBlock2Row"),
      mIDCT2D("mIDCT2D"),
      mCol2Block("mCol2Block"),
      // End IDCT2D
//    mRound("mRound"),
      mInvLevel("mInvLevel"),
      mClip("mClip"),
      mSink("mSink")
  {
    connectNodePorts<64>(mIDCTScanSource.out, mBlock2Row.b);
    
    connectNodePorts<16>(mBlock2Row.C0, mIDCT2D.i0);
    connectNodePorts<16>(mBlock2Row.C1, mIDCT2D.i1);
    connectNodePorts<16>(mBlock2Row.C2, mIDCT2D.i2);
    connectNodePorts<16>(mBlock2Row.C3, mIDCT2D.i3);
    connectNodePorts<16>(mBlock2Row.C4, mIDCT2D.i4);
    connectNodePorts<16>(mBlock2Row.C5, mIDCT2D.i5);
    connectNodePorts<16>(mBlock2Row.C6, mIDCT2D.i6);
    connectNodePorts<16>(mBlock2Row.C7, mIDCT2D.i7);
    
    connectNodePorts<16>(mIDCT2D.o0, mCol2Block.R0);
    connectNodePorts<16>(mIDCT2D.o1, mCol2Block.R1);
    connectNodePorts<16>(mIDCT2D.o2, mCol2Block.R2);
    connectNodePorts<16>(mIDCT2D.o3, mCol2Block.R3);
    connectNodePorts<16>(mIDCT2D.o4, mCol2Block.R4);
    connectNodePorts<16>(mIDCT2D.o5, mCol2Block.R5);
    connectNodePorts<16>(mIDCT2D.o6, mCol2Block.R6);
    connectNodePorts<16>(mIDCT2D.o7, mCol2Block.R7);
    
//  connectNodePorts<64>(mCol2Block.b, mRound.in);
//  connectNodePorts<1>(mRound.out, mInvLevel.in);
    connectNodePorts<64>(mCol2Block.b,  mInvLevel.in);
    connectNodePorts<1>(mInvLevel.out, mClip.in);
    connectNodePorts<1>(mClip.out,     mSink.in);
    
    connectNodePorts<1>(mIDCTScanSource.outCtrlImage, mSink.inCtrlImage);
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  if (argc <= 3) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <width> <height> <scanpattern:idctcoeff filename>+" << std::endl;
    exit(-1);
  }
  
  size_t      width, height;
  ScanVector  scanVector;
  
  width  = atoi(argv[1]);
  height = atoi(argv[2]);
  
  for (const char *const *argIter = &argv[3]; *argIter != NULL; ++argIter) {
    size_t      pos = 0;
    const char *arg = *argIter;
    
    Scan scan;

    while (pos < SCANPATTERN_LENGTH) {
      if (arg[pos] < '0' || arg[pos] > '2') {
        std::cerr << argv[0] << ": scanpattern format error, scanpattern: [0-2]{6} !" << std::endl;
        exit (-1);
      }
      scan.scanPattern[pos] = arg[pos++] - '0';
    }
    if (arg[pos++] != ':') {
      std::cerr << argv[0] << ": missing colon after scanpattern !" << std::endl;
    }
    scan.idctCoeffFileName = &arg[pos];
    scanVector.push_back(scan);
  }
  
  smoc_top_moc<Testbench> testbench("testbench", width, height, scanVector);
  
  sc_start(-1);
  
  return 0;
}
#endif