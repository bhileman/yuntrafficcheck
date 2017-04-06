import urllib2
import json
import sys

response = urllib2.urlopen('URL HERE')
data = json.load(response)
ttime = data['rows'][0]['elements'][0]['duration_in_traffic']['value']
print (ttime)

sys.exit()
