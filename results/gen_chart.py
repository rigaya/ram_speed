# -*- coding: utf-8 -*-
import sys
import os
import re
import colorcet as cc
import datetime

from enum import Enum
class DataType(Enum):
    Unknown = 0
    BandwidthRead = 1
    BandwidthWrite = 2
    Latency = 3
    InterCoreLatency = 4

class BenchmarkDataInterCoreLatencyData:
    core_x = -1
    core_y = -1
    latency = 0.0
    def __init__(self, core_x_, core_y_, latency_):
        self.core_x = core_x_
        self.core_y = core_y_
        self.latency = latency_
        
    def print_data(self):
        print('  latency core ' + str(self.core_x) + '-' + str(self.core_y) + ': ' + str(self.latency))

class BenchmarkDataInterCoreLatency:
    values = None
    def __init__(self):
        self.values = []

    def print_data(self):
        print('-- inter core latency -------------------')
        for data in self.values:
            data.print_data()

class BenchmarkDataLatencyData:
    size = 0
    dict_values = None
    
    def __init__(self, size_, dict_values):
        self.size = size_
        self.dict_values = dict_values
        
    def print_data(self):
        str_data = '  size ' + str(self.size) + ': '
        for type, value in self.dict_values.items():
            str_data = str_data + ', ' + str(value)
        print(str_data)

class BenchmarkDataLatency:
    list_size_latency = None
    def __init__(self):
        self.list_size_latency = []

    def print_data(self):
        print('-- ram latency -------------------')
        for data in self.list_size_latency:
            data.print_data()

class BenchmarkDataBandwidthData:
    size = 0
    bandwidth_data = None
    def __init__(self, size_, bandwidth_data_):
        self.size = size_
        self.bandwidth_data = bandwidth_data_
        
    def print_data(self):
        str_data = '  size ' + str(self.size) + ': '
        for value in self.bandwidth_data:
            str_data = str_data + ', ' + str(value)
        print(str_data)

class BenchmarkDataBandwidth:
    list_size_data = None
    def __init__(self):
        self.list_size_data = []

    def print_data(self):
        print('-- bandwdith -------------------')
        for data in self.list_size_data:
            data.print_data()

class BenchmarkData:
    name = ''
    hidden = False
    color = ''
    dataInterCoreLatency = None
    dataLatency = None
    dataBandwidth = None

    def __init__(self, name_, hidden_, color_):
        self.name = name_
        self.hidden = hidden_
        self.color = color_
        self.dataInterCoreLatency = BenchmarkDataInterCoreLatency()
        self.dataLatency = BenchmarkDataLatency()
        self.dataBandwidth = dict()
        self.dataBandwidth[DataType.BandwidthRead]  = BenchmarkDataBandwidth()
        self.dataBandwidth[DataType.BandwidthWrite] = BenchmarkDataBandwidth()

    def read(self, filename):
        read_mode = DataType.Unknown
        inter_core_latency_header = None
        inter_core_latency_row = 0
        latency_header = None
        print('read from file ' + filename, file=sys.stderr)
        with open(filename,'r', encoding='utf-8') as f:
            for line in f.readlines():
                line = line.rstrip()
                if 'inter core latency' in line:
                    read_mode = DataType.InterCoreLatency
                elif 'ram latency' in line:
                    read_mode = DataType.Latency
                elif 'read bandwidth' in line:
                    read_mode = DataType.BandwidthRead
                elif 'write bandwidth' in line:
                    read_mode = DataType.BandwidthWrite
                elif len(line) == 0:
                    read_mode = DataType.Unknown
                elif read_mode == DataType.InterCoreLatency:
                    value = line.split(',')
                    is_header = False
                    try:
                        a = float(value[1])
                    except:
                        is_header = True
                        
                    if inter_core_latency_header is None and is_header and inter_core_latency_row == 0:
                        inter_core_latency_header = line.split(',')
                        inter_core_latency_row = 0
                    else:
                        istart = 1 if not inter_core_latency_header is None else 0
                        for i in range(istart, len(value)):
                            if len(value[i]) > 0:
                                core_x = int(inter_core_latency_header[i]) if not inter_core_latency_header is None else i
                                core_y = inter_core_latency_row
                                try:
                                    latency = float(value[i])
                                    self.dataInterCoreLatency.values.append(BenchmarkDataInterCoreLatencyData(core_x, core_y, latency))
                                except:
                                    pass

                        inter_core_latency_row += 1
                        
                elif read_mode == DataType.Latency:
                    if latency_header is None:
                        latency_header = line.split(',')
                    else:
                        value = line.split(',')
                        size = int(value[0])
                        size_value = dict()
                        for i in range(1, len(latency_header)):
                            size_value[latency_header[i].strip()] = value[i]
                        
                        self.dataLatency.list_size_latency.append(BenchmarkDataLatencyData(size, size_value))
                        
                elif read_mode == DataType.BandwidthRead or read_mode == DataType.BandwidthWrite:
                    value = line.split(',')
                    size = int(value[0])
                    list_bandwidth = []
                    for str_bandwidth in value[1:]:
                        if len(str_bandwidth) > 0:
                            list_bandwidth.append(float(str_bandwidth))
                    self.dataBandwidth[read_mode].list_size_data.append(BenchmarkDataBandwidthData(size, list_bandwidth))


    def print_data(self, output_file):
        print('----  ' + self.name + '  ------------------', file = output_file)
        self.dataInterCoreLatency.print_data()
        self.dataLatency.print_data()
        self.dataBandwidth[DataType.BandwidthRead].print_data()
        self.dataBandwidth[DataType.BandwidthWrite].print_data()

def get_scatter_header(name):
    header = 'var ctx_' + name + '_benchmark = document.getElementById(\'chart_' + name + '\');\n'
    header = header + 'const chart_' + name + '_benchmark = new Chart(ctx_' + name + '_benchmark,\n'
    header = header + '{\n'
    header = header + '  type: \'scatter\',\n'
    return header

def get_scatter_bandwdith_st(list_benchmark_data, mode):
    data = ''
    data = data + '  data: {\n'
    data = data + '    datasets: [\n'
    
    dataset = ''
    for benchmark_data in list_benchmark_data:
        color = benchmark_data.color
        hidden = 'true' if benchmark_data.hidden else 'false'
        dataset = dataset + '      {\n'
        dataset = dataset + '        label: \"' + benchmark_data.name + "\",\n"
        dataset = dataset + '        showLine: true,\n'
        dataset = dataset + '        hidden: ' + hidden + ',\n'
        dataset = dataset + '        lineTension: 0.4,\n'
        dataset = dataset + '        borderWidth: 2,\n'
        dataset = dataset + '        backgroundColor: \'rgba(255, 255, 255, 0.0)\',\n'
        dataset = dataset + "        borderColor: 'rgba(" + str(int(color[0]*255)) + ", " + str(int(color[1]*255)) + ", " + str(int(color[2]*255)) + ", 0.9)',\n"
        dataset = dataset + '        data: [\n'
        for bandwidth_data in benchmark_data.dataBandwidth[mode].list_size_data:
            dataset = dataset + '            { x: ' + str(bandwidth_data.size) + ', y: ' + str(bandwidth_data.bandwidth_data[0]) + ' },\n'
        dataset = dataset + '        ],\n'
        dataset = dataset + '      },\n'

    data = data + dataset + '    ],\n'
    data = data + '  },\n'
    
    return data

def get_scatter_bandwdith_mt(list_benchmark_data, mode):
    data = ''
    data = data + '  data: {\n'
    data = data + '    datasets: [\n'
    
    
    dataset = ''
    for benchmark_data in list_benchmark_data:
        color = benchmark_data.color
        hidden = 'true' if benchmark_data.hidden else 'false'
        dataset = dataset + '      {\n'
        dataset = dataset + '        label: \"' + benchmark_data.name + "\",\n"
        dataset = dataset + '        showLine: true,\n'
        dataset = dataset + '        hidden: ' + hidden + ',\n'
        dataset = dataset + '        lineTension: 0.4,\n'
        dataset = dataset + '        borderWidth: 2,\n'
        dataset = dataset + '        backgroundColor: \'rgba(255, 255, 255, 0.0)\',\n'
        dataset = dataset + "        borderColor: 'rgba(" + str(int(color[0]*255)) + ", " + str(int(color[1]*255)) + ", " + str(int(color[2]*255)) + ", 0.9)',\n"
        dataset = dataset + '        data: [\n'
        
        max_cores = 0
        for bandwidth_data in benchmark_data.dataBandwidth[mode].list_size_data:
            max_cores = max(max_cores, len(bandwidth_data.bandwidth_data))
        
        for bandwidth_data in benchmark_data.dataBandwidth[mode].list_size_data:
            if len(bandwidth_data.bandwidth_data) == max_cores:
                dataset = dataset + '            { x: ' + str(bandwidth_data.size) + ', y: ' + str(bandwidth_data.bandwidth_data[max_cores-1]) + ' },\n'
        dataset = dataset + '        ],\n'
        dataset = dataset + '      },\n'

    data = data + dataset + '    ],\n'
    data = data + '  },\n'
    
    return data

def get_scatter_bandwdith_threads(list_benchmark_data, mode):
    data = ''
    data = data + '  data: {\n'
    data = data + '    datasets: [\n'
    
    dataset = ''
    for benchmark_data in list_benchmark_data:
        if len(benchmark_data.dataBandwidth[mode].list_size_data) == 0:
            continue

        color = benchmark_data.color
        hidden = 'true' if benchmark_data.hidden else 'false'
        dataset = dataset + '      {\n'
        dataset = dataset + '        label: \"' + benchmark_data.name + "\",\n"
        dataset = dataset + '        showLine: true,\n'
        dataset = dataset + '        hidden: ' + hidden + ',\n'
        dataset = dataset + '        lineTension: 0.4,\n'
        dataset = dataset + '        borderWidth: 2,\n'
        dataset = dataset + '        backgroundColor: \'rgba(255, 255, 255, 0.0)\',\n'
        dataset = dataset + "        borderColor: 'rgba(" + str(int(color[0]*255)) + ", " + str(int(color[1]*255)) + ", " + str(int(color[2]*255)) + ", 0.9)',\n"
        dataset = dataset + '        data: [\n'
        
        max_size_bandwidth_data = benchmark_data.dataBandwidth[mode].list_size_data[-1]
        for threads in range(1, len(max_size_bandwidth_data.bandwidth_data)+1):
            bandwidth_data = max_size_bandwidth_data.bandwidth_data[threads-1]
            dataset = dataset + '            { x: ' + str(threads) + ', y: ' + str(bandwidth_data) + ' },\n'
        dataset = dataset + '        ],\n'
        dataset = dataset + '      },\n'

    data = data + dataset + '    ],\n'
    data = data + '  },\n'
    
    return data

def get_scatter_latency(list_benchmark_data, target):
    data = ''
    data = data + '  data: {\n'
    data = data + '    datasets: [\n'
    
    dataset = ''
    for benchmark_data in list_benchmark_data:
        color = benchmark_data.color
        hidden = 'true' if benchmark_data.hidden else 'false'
        dataset = dataset + '      {\n'
        dataset = dataset + '        label: \"' + benchmark_data.name + "\",\n"
        dataset = dataset + '        showLine: true,\n'
        dataset = dataset + '        hidden: ' + hidden + ',\n'
        dataset = dataset + '        lineTension: 0.4,\n'
        dataset = dataset + '        borderWidth: 2,\n'
        dataset = dataset + '        backgroundColor: \'rgba(255, 255, 255, 0.0)\',\n'
        dataset = dataset + "        borderColor: 'rgba(" + str(int(color[0]*255)) + ", " + str(int(color[1]*255)) + ", " + str(int(color[2]*255)) + ", 0.9)',\n"
        dataset = dataset + '        data: [\n'
        for latency_data in benchmark_data.dataLatency.list_size_latency:
            dataset = dataset + '            { x: ' + str(latency_data.size) + ', y: ' + str(latency_data.dict_values[target]) + ' },\n'
        dataset = dataset + '        ],\n'
        dataset = dataset + '      },\n'

    data = data + dataset + '    ],\n'
    data = data + '  },\n'
    
    return data

def get_chart_options(name, aspect_ratio, x_label, x_log, x_min, y_label, y_log, y_min):
    options = r'''
  options: {
    // レスポンシブ対応
    responsive: true,
    maintainAspectRatio: true,
    aspectRatio: $ASPECT_RATIO$,
    locale: 'ja-JP',
    elements: {
        point:{
            radius: 1
        }
    },
    plugins: {
      // グラフタイトルの設定
      title: {
        text: '$GRAPH_TITLE$',
        display: true,
        font: {
          size: 16,
        }
      },
      // 凡例の設定
      legend: {
        display: true,
        position: 'top',
        align: 'center',
        labels: {
          fontSize: 12,
        }
      },
      tooltip: {
        callbacks: {
          label: function(context) {
            var value = context.parsed;
            return context.dataset.label + ': $X_LABEL$ = ' + value.x + ', $Y_LABEL$ = ' + value.y;
          },
        },
      },
    },
    // x軸,y軸の設定
    scales: {
      x: {
        type: '$X_TYPE$',
        display: true,
        position: 'bottom',
        min: $X_AXIS_MIN$,
        //max: $X_AXIS_MAX$,
        title: {
          display: true,
          text: '$X_LABEL$',
          font: {
            size: 12,
          }
        },
      },
      y: {
        type: '$Y_TYPE$',
        display: true,
        min: $Y_AXIS_MIN$,
        //max: $Y_AXIS_MAX$,
        title: {
          display: true,
          text: '$Y_LABEL$',
          font: {
            size: 12,
          }
        },
      }
    },
  },
'''
    options = options.replace('$ASPECT_RATIO$', aspect_ratio)
    options = options.replace('$GRAPH_TITLE$', name)
    #options = options.replace('$X_AXIS_MAX$', str(x_max))
    options = options.replace('$X_LABEL$', x_label)
    options = options.replace('$Y_LABEL$', y_label)
    options = options.replace('$X_AXIS_MIN$', str(x_min))
    options = options.replace('$Y_AXIS_MIN$', str(y_min))
    if x_log:
        options = options.replace('$X_TYPE$', 'log2')
    else:
        options = options.replace('$X_TYPE$', 'linear')
    if y_log:
        options = options.replace('$Y_TYPE$', 'logarithmic')
    else:
        options = options.replace('$Y_TYPE$', 'linear')
    return options

def get_chart_footer():
    return r'''
});
'''

def get_date_prefix():
    tdatetime = datetime.datetime.now()
    tstr = tdatetime.strftime('%Y%m%d')
    return tstr

def create_latency_cacheline_scatter_chart(list_benchmark_data):
    return get_scatter_header(get_date_prefix() + '_ram_latency_cacheline') + \
        get_scatter_latency(list_benchmark_data, 'cacheline forward2') + \
        get_chart_options('Latency (cacheline)', 'aspect_ratio_latency', 'data size [KB]', True, 4, 'latency [ns]', True, 1) + \
        get_chart_footer()

def create_latency_full_random_scatter_chart(list_benchmark_data):
    return get_scatter_header(get_date_prefix() + '_ram_latency_full_random') + \
        get_scatter_latency(list_benchmark_data, 'full random') + \
        get_chart_options('Latency (full random)', 'aspect_ratio_latency', 'data size [KB]', True, 4, 'latency [ns]', True, 1) + \
        get_chart_footer()

def create_bandwdith_threads_scatter_chart(list_benchmark_data):
    return get_scatter_header(get_date_prefix() + '_ram_bandwidth_threads') + \
        get_scatter_bandwdith_threads(list_benchmark_data, DataType.BandwidthRead) + \
        get_chart_options('RAM bandwidth', 'aspect_ratio_bandwidth_threads', 'threads', False, 0, 'bandwidth [GB/s]', False, 0) + \
        get_chart_footer()


def create_bandwdith_st_scatter_chart(list_benchmark_data):
    return get_scatter_header(get_date_prefix() + '_ram_bandwidth_st') + \
        get_scatter_bandwdith_st(list_benchmark_data, DataType.BandwidthRead) + \
        get_chart_options('Bandwidth (Single Thread)', 'aspect_ratio_bandwidth', 'data size [KB]', True, 4, 'bandwidth [GB/s]', True, 10) + \
        get_chart_footer()

def create_bandwdith_mt_scatter_chart(list_benchmark_data):
    return get_scatter_header(get_date_prefix() + '_ram_bandwidth_mt') + \
        get_scatter_bandwdith_mt(list_benchmark_data, DataType.BandwidthRead) + \
        get_chart_options('Bandwidth (Multi Thread)', 'aspect_ratio_bandwidth', 'data size [KB]', True, 4, 'bandwidth [GB/s]', True, 10) + \
        get_chart_footer()
        
def aspect_ratios():
    return r'''
var aspect_ratio_latency = 0.8;
var aspect_ratio_bandwidth = 0.9;
var aspect_ratio_bandwidth_threads = 1.0;
'''

if __name__ == '__main__':
    list_benchmark_data = []
    
    color_list = cc.glasbey_bw_minc_20_minl_30
    
    iarg = 1

    for i in range(iarg, len(sys.argv)):
        benchmark_entry = sys.argv[i].split(',')
        benchmark_entry_name   = benchmark_entry[0]
        benchmark_entry_hidden = benchmark_entry[1] == 'hidden'
        if len(benchmark_entry) == 4:
            benchmark_entry_file   = benchmark_entry[3]
            color = benchmark_entry[2].lstrip('#')
            benchmark_entry_color = tuple(float(int(color[i:i+2], 16))/255.0 for i in (0, 2, 4))
        else:
            benchmark_entry_file   = benchmark_entry[2]
            benchmark_entry_color  = color_list[i]

        benchmark_data = BenchmarkData(benchmark_entry_name, benchmark_entry_hidden, benchmark_entry_color)
        benchmark_data.read(benchmark_entry_file)
        #benchmark_data.print_data(sys.stderr)
        
        list_benchmark_data.append(benchmark_data)
        
    print(aspect_ratios())
    print()
    print(create_latency_cacheline_scatter_chart(list_benchmark_data))
    print()
    print(create_latency_full_random_scatter_chart(list_benchmark_data))
    print()
    print(create_bandwdith_st_scatter_chart(list_benchmark_data))
    print()
    print(create_bandwdith_mt_scatter_chart(list_benchmark_data))
    print()
    print(create_bandwdith_threads_scatter_chart(list_benchmark_data))
    print()