#!-*- coding:utf8-*-

import sys
import getopt
import networkx as nx
import matplotlib.pyplot as plt
import openpyxl as xls


def main(argv):
    xlsx = "../Conf/services.xlsx"
    sheet = "one"
    name = "logic"

    try:
        opts, _ = getopt.getopt(
            argv, "hx:s:n", ["xlsx=", "sheet=", "name="])
    except getopt.GetoptError:
        print('Draw.py -x <xlsx> -s <sheet> -n <name>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print('Draw.py -x <xlsx> -s <sheet> -n <name>')
            sys.exit()
        elif opt in ("-x", "--xlsx"):
            xlsx = arg
        elif opt in ("-s", "--sheet"):
            sheet = arg
        elif opt in ("-n", "--name"):
            name = arg

    if xlsx == "" or sheet == "" or name == "":
        sys.exit()

    print(xlsx, sheet, name)

    wb = xls.load_workbook(xlsx)
    sh = wb[sheet]

    mm = dict()
    for cases in list(sh.rows):
        svr = str(cases[0].value)
        subscribe_list = str(cases[1].value)
        if str(svr) == "None":
            break
        mm[svr] = subscribe_list.split()
        # print(svr)
        # print(subscribe_list.split())
    wb.close()
    # print(mm)

    graph = nx.DiGraph()
    s = dict()
    dfs(name, mm, graph, s)
    nx.draw_networkx(graph, None, True, True)
    # plt.savefig(name + ".svg")
    plt.show()


def dfs(name, mm, graph, s):
    if name == "":
        return

    if name in s.keys():
        return
    else:
        s[name] = 1

    graph.add_node(name)
    if name not in mm.keys():
        return

    subscribe_list = mm[name]

    if len(subscribe_list) == 0:
        return

    for svr in subscribe_list:
        if svr == "" or svr == name:
            continue
        graph.add_edge(name, svr)
        print(name, "->", svr)
        dfs(svr, mm, graph, s)


if __name__ == '__main__':
    main(sys.argv[1:])
