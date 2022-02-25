import os
import collections

root_path = os.path.join(os.getcwd(), "..")
def gen_city_scripts():
    gqls = []
    # {create: 'graph1', index: 'keyword'}
    gqls.append("{create: 'city'}")
    cities_name = []
    idx = 1
    for line in open(root_path + '/data/provinces', encoding="utf8"):
        cities_name.append(line.strip())
        idx += 1
    cities = '['
    for idx in range(len(cities_name)):
        cities += "['" + str(idx + 1) + "', {name: '" + cities_name[idx] + "'}],"
    cities = cities.strip(',') + ']'
    gqls.append("{upset: 'city', vertex: " + cities)

    connections = {}
    for line in open(root_path + '/data/provinces_link', encoding="utf8"):
        line = line.strip()
        pair = line.split(': ')
        name = pair[0]
        near_cities = pair[1].split(',')
        connections[name] = near_cities

    with open('get_near_province.ngl', 'w', encoding="utf8") as f:
        for line in gqls:
            f.write(line + '\n')

if __name__ == "__main__":
    gen_city_scripts()
