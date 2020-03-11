#!/usr/bin/env python3
from bs4 import BeautifulSoup
from parse import parse
html = open("isotopic-abundances.htm").read()
soup = BeautifulSoup(html, features="html.parser", multi_valued_attributes=None)
table = soup.find("table")

output_rows = []
Z=0

for table_row in table.findAll('tr'):
    child=False
    columns = table_row.findAll('td')
    abundance_single = 0.0
    abundance_low = 0.0
    abundance_high = 0.0
#    print(columns)
    if(len(columns) < 2):
        continue
    
    if str(table_row.get('class')) == "tablesorter-childRow":
#        print("This is a child")
        A=int(columns[0].text)
        abundance=columns[1].text.strip()
        child=True
    else:
        try:
            A=int(columns[3].text)
            Z=int(columns[0].text)
        except ValueError:
            continue
        abundance=columns[4].text.strip()
#        print("This is not a child")
    abundance=abundance.replace(" ", "");
    abundance=abundance.replace(" ", "");
#    if abundance[0]=='[':
#       p=parse("[{:f},{:f}]", abundance)
#       if not p:
#        continue
#       abundance_low=p[0]
#       abundance_high=p[1]
#       print(f"got low {abundance_low:f} and high {abundance_high:f}")
    p=parse("{:g}", abundance)
    if p:
        abundance_single=p[0]
    p=parse("{:f}({d})", abundance)
    if p:
        abundance_single=p[0]
    p=parse("[{:f},{:f}]", abundance)
    if p:
       abundance_low=p[0]
       abundance_high=p[1]
       abundance_single=(p[0]+p[1])/2 #TODO: not the greatest choice?

    print(f"{Z:3d} {A:3d} {abundance_single:5f} {abundance_low:5f} {abundance_high:5f}")
