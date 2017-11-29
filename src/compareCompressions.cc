
#include "compressedSetBit.h"
#include "bitvector.h"


int main(int argc, char *argv[]) {

  uint16_t num_samples = 2586;
  size_t gtBV = 0;
  size_t gtBVR = 0;
  size_t bvrGTbv = 0;
  size_t bvEQbvr = 0;
  size_t compressedSum = 0;
  size_t bvSum = 0;
  size_t bvrSum = 0;

  size_t roundIdxCnt = 0;
  size_t totalIdxCnt = 0;
  std::string filename = argv[1];
  BitVectorRRR eqcls(filename);
  size_t totalEqClsCnt = eqcls.bit_size()/num_samples; //222584822;
  std::cout << "Total bit size: " << eqcls.bit_size() << "\ntotal # of equivalence classes: " << totalEqClsCnt << "\n";
  for (size_t eqclsCntr = 0; eqclsCntr < totalEqClsCnt; eqclsCntr++) {
    BitVector bv(num_samples);
    std::vector<uint32_t> idxList;
    idxList.reserve(num_samples);
    size_t i = 0;
    while (i < num_samples) {
      size_t bitCnt = std::min(num_samples-i, (size_t)64);
      int wrd = eqcls.get_int(eqclsCntr*num_samples+i, bitCnt);
      for (size_t j = 0, curIdx = i; j < bitCnt; j++, curIdx++) {
        if ((wrd >> j) & 0x01) {
          bv.set(curIdx);
          idxList.push_back(curIdx);
        }
      }
      i+=bitCnt;
    }
    totalIdxCnt += idxList.size();
    roundIdxCnt += idxList.size();
    if (eqclsCntr != 0 && (eqclsCntr) % 1000000 == 0) {
      std::cout << "\n\nTotal number of experiments are : " << eqclsCntr <<
        "\nTotal average set bits: " << totalIdxCnt /eqclsCntr <<
        "\nThis round average set bits: " << roundIdxCnt/1000000 <<
                                                        "\nbv average size : " << bvSum/eqclsCntr <<
                                                        "\nbvr average size : " << bvrSum/eqclsCntr <<
                                                        "\ncompressed average size : " << compressedSum/eqclsCntr <<
                                                        "\ncompressed > bv : " << gtBV << " or " << (gtBV*100)/eqclsCntr << "% of the time" <<
                                                        "\ncompressed > bvr : " << gtBVR << " or " << (gtBVR*100)/eqclsCntr << "% of the time" <<
                                                        "\nbvr > bv : " << bvrGTbv << " or " << (bvrGTbv*100)/eqclsCntr << "% of the time" <<
                                                        "\n";
      roundIdxCnt = 0;
    }

    BitVectorRRR bvr(bv);
    CompressedSetBit setBitList(idxList);

    size_t bvSize = bv.size_in_bytes();
    size_t bvrSize = bvr.size_in_bytes();
    size_t compressedSize = setBitList.size_in_bytes();
    bvSum += bvSize;
    bvrSum += bvrSize;
    compressedSum += compressedSize;
    if (compressedSize > bvSize) gtBV++;
    if (compressedSize > bvrSize) gtBVR++;
    if (bvrSize > bvSize) bvrGTbv++;
    if (bvrSize == bvSize) bvEQbvr++;
    //vector<uint32_t> output;
    //setBitList.uncompress(output);
  }
  std::cout << "\n\n\nFinalResults:\n" <<
    "Total number of experiments are : " << totalEqClsCnt << "\n" <<
    "\nbv average size : " << bvSum/totalEqClsCnt <<
    "\nbvr average size : " << bvrSum/totalEqClsCnt <<
    "\ncompressed average size : " << compressedSum/totalEqClsCnt <<
    "\nHow many times compressed > bv : " << gtBV << " or " << (gtBV*100)/totalEqClsCnt << "% of the time" <<
    "\nHow many times compressed > bvr : " << gtBVR << " or " << (gtBVR*100)/totalEqClsCnt << "% of the time" <<
    "\nHow many times bvr > bv : " << bvrGTbv << " or " << (bvrGTbv*100)/totalEqClsCnt << "% of the time" <<
    "\nHow many times bvr == bv : " << bvEQbvr <<  " or " << (bvEQbvr*100)/totalEqClsCnt << "% of the time" <<
    "\n";
}