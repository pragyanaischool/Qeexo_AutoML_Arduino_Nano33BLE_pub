# Used in conjuction with white source CI

import json
import os

import requests


def send_request():
    # Request
    # POST https://app.whitesourcesoftware.com/api/v1.3

    try:
        response = requests.post(
            url='https://app.whitesourcesoftware.com/api/v1.3',
            headers={
                'Accept-Charset': 'UTF-8',
                'Content-Type': 'application/json',
                'Cookie': 'JSESSIONID=2xab3xIg3bF7kfU97od7zlWe1ZnTpDp6Fu1veoFY.api-01',
            },
            data=json.dumps({
                'productToken': '913e1446e73844588179f565f65fa3f19bd27c2facdf477ba1552574e44436e9',
                'userKey': os.environ['WHITESOURCE_USER_KEY'],
                'excludeProjectOccurrences': False,
                'requestType': 'getProductLicenses'
            })
        )
        return json.loads(response.content.decode('utf-8'))
    except requests.exceptions.RequestException:
        print('HTTP Request failed')


data = send_request()

for library in data.get('libraries'):
    library.pop('keyUuid', None)
    library.pop('keyId', None)
    library.pop('directDependency', None)
    library.pop('sha1', None)
    library.pop('artifactId', None)

print(json.dumps(data))
