import sys
from pathlib import Path
import argparse
from lxml import etree as ET

parser = argparse.ArgumentParser()
parser.add_argument('OUTP_FILE', help='Path to .qrc file')
parser.add_argument('--ext', required=True)
parser.add_argument('--root', required=True, help='root folder for search')
parser.add_argument('-r', action='store_true', help='recursive')

args = vars(parser.parse_args())

root = Path(args['root'])
out_path = Path(args['OUTP_FILE'])
extensions = args['ext'].split(' ')
if args['r'] is not None:
    rec = True

def parse_folder(folder: Path, extensions: list[str], recursive=False) -> list[Path]:
    ret = []
    for f in folder.iterdir():
        if f.is_file() and f.suffix in extensions:
            ret.append(f)
        elif recursive and f.is_dir():
            ret += parse_folder(f, extensions, recursive)
    return ret

with out_path.open('wb') as of:
    path_list = parse_folder(root, extensions, rec)
    
    qrc_root = ET.Element('RCC')
    resources = ET.SubElement(qrc_root, 'qresource', prefix="/")
    # add resources
    for p in path_list:
        ET.SubElement(resources, 'file').text = p.as_posix()

    of.write(ET.tostring(qrc_root, pretty_print=True, encoding='utf-8'))