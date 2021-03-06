import os
from dataset import Dataset
import argparse

if __name__ == '__main__':

    parser = argparse.ArgumentParser(prog="balancing")
    parser.add_argument('--read', type=str, default="./",help='files path')
    parser.add_argument('--chunk', type=int, default="50",help='chunk size')
    parser.add_argument('--offset', type=int, default="0",help='offset size')
    parser.add_argument('--limit', type=int, default="10000",help='offset size')
    parser.add_argument('--pdg',action='store_true')
    parser.add_argument('--det',action='store_true')
    parser.add_argument('--lab',action='store_true')
    parser.add_argument('--all',action='store_true')
    args = parser.parse_args()

    remote_data = args.read + "/"
    chunksize   = args.chunk
    offset      = args.offset
    limit       = args.limit
    #remote_data = "data/inference/unzip/"
    new_dir = remote_data + "/det_data/"

    files = [remote_data + el for el in os.listdir(remote_data)]

    if not os.path.exists(new_dir):
        os.makedirs(new_dir)

    for chunk in  range(offset,int(((len(files) + chunksize))/chunksize) + 1):
        if(min(len(files),chunk*chunksize)==min(len(files),chunksize*(chunk+1)) and chunk!=0):
            continue

        if chunk > offset + limit:
            break

        if args.debug:
            p = files[:2]
        else:
            p = files[min(len(files),chunk*chunksize):min(len(files),chunksize*(chunk+1))]

        data = Dataset(p)
        print("loading & balancing data...")

        name = "/"

        if args.all:
            data = data.balance_by_det().balance_by_pdg().balance_data()
            name = name + "det_pdg_bal_"
        else:
            if args.det:
                data = data.balance_by_det()
                name = name + "det_"
            if args.pdg:
                data = data.balance_by_pdg()
                name = name + "pdg_"
            if args.lab:
                data = data.balance_data()
                name = name + "det_pdg_bal_"

        print("dumping data...")
        print("data size %d" + str(data.data.shape[0]))

        name = name + "dataset_" + str(chunk) + ".h5"
        data.save(remote_data + name)
