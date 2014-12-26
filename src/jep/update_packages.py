#!/usr/bin/env python

import requests
from bs4 import BeautifulSoup


for major_version in (6, 7, 8,):
    r = requests.get('http://docs.oracle.com/javase/{}/docs/api/allclasses-frame.html'.format(major_version))
    r.raise_for_status()
    soup = BeautifulSoup(r.content)

    with open('classlist_{}.txt'.format(major_version), 'wb') as f:
        for anchor in soup.find_all('a'):
            name = anchor['href'].lstrip('/').replace('.html', '') + '\n'
            if '#' in name:
                continue
            f.write(name)
