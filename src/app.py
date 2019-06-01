import json
import pandas as pd
import urllib
import requests
from flask import Flask, render_template, request, redirect, Response
from flask import jsonify, url_for

app = Flask(__name__)

@app.route('/query', methods=['POST'])
def process_query():
    req_data = json.loads(request.data)
    response = {'message':'OK','data':'','status':'200'}
    #build the query using the parameters passed
    url = 'http://localhost:9080/query/'
    url += urllib.quote(req_data['query_prefix'], safe='')
    url += '/'
    url += urllib.quote(req_data['output'], safe='')
    url += '/'
    url += urllib.quote(req_data['input'], safe='')
    #send the query make a post call
    import pdb;pdb.set_trace()
    resp = requests.post(url)
    #retrieve the response and send the response back to the caller
    response['data'] = resp.text
    return json.dumps(response)