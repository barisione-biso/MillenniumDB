import json
import sys
from functools import reduce
from milleniumdb import MilleniumDB


# Parse file each line at a time
def parse_lines(file):
    for line in file:
        clean_line = line.strip()
        start = clean_line.find('{')
        if start >= 0:
            end = clean_line.rfind('}') + 1
            yield clean_line[start:end]


# Parse snaks
def parse_snak(subject, snak, graph, rank=None, c_type='Statement'):
    # rank_string = ''
    c_object = None
    to_object = ''
    data_type = 'Not Specified'
    if 'datatype' in snak:
        data_type = snak['datatype']
    # if rank:
    #     rank_string = f', rank: {rank}'
    if snak['snaktype'] == 'value':
        value = snak['datavalue']
        v_type = value['type']
        if v_type == 'wikibase-entityid':
            if 'id' in value['value']:
                raw_id = value['value']['id']
                to_object = raw_id
                if '-' in raw_id:
                    to_object = raw_id.replace('-', '_')
            elif 'numeric-id' in value['value']:
                if data_type == 'wikibase-item':
                    to_object = f'Q{value["value"]["numeric-id"]}'
                elif data_type == 'wikibase-property':
                    to_object = f'P{value["value"]["numeric-id"]}'
                else:
                    # TODO: print datatype only once
                    print("unknown datatype:", data_type)
                    to_object = f'OTHER_DATATYPE{value["value"]["numeric-id"]}'
        elif v_type == 'string':
            a_string = graph.define_anon(
                labels=['String'],
                properties=[('string_value', value['value'])])
            a_string.save()

            c_object = a_string
        elif v_type == 'globecoordinate':
            a_globe = graph.define_anon(
                labels=['Globecoordinate'],
                properties=[('latitude', value['value']['latitude']),
                            ('longitude', value['value']['longitude']),
                            ('precision', value['value']['precision']),
                            ('globe', value['value']['globe'])])
            a_globe.save()
            c_object = a_globe
        elif v_type == 'monolingualtext':
            a_mono = graph.define_anon(
                labels=['MonolingualText'],
                properties=[('language', value['value']['language']),
                            ('string_value', value['value']['text'])])
            a_mono.save()
            c_object = a_mono
        elif v_type == 'quantity':
            q_props = [('amount', value['value']['amount']),
                       ('unit', value['value']['unit'])]
            # upper_string = ''
            # lower_string = ''
            if 'upperBound' in value['value']:
                # upper_string = f'upperBound: {value["value"]["upperBound"]}, '
                q_props.append(('upperBound', value['value']['upperBound']))
            if 'lowerBound' in value['value']:
                # lower_string = f'lowerBound: {value["value"]["lowerBound"]}, '
                q_props.append(('lowerBound', value['value']['lowerBound']))
            a_quantity = graph.define_anon(
                labels=['Quantity'],
                properties=q_props)
            a_quantity.save()

            c_object = a_quantity
        elif v_type == 'time':
            a_time = graph.define_anon(
                labels=['Time'],
                properties=[('time', value['value']['time']),
                            ('timezone', value['value']['timezone']),
                            ('calendarmodel', value['value']['calendarmodel']),
                            ('precision', value['value']['precision'])])
            a_time.save()
            c_object = a_time
        if c_object:
            to_object = c_object.identifier
        else:
            c_object = str(to_object)
        return subject.add_connection(
            c_object, types=[snak['property']],
            properties=[('snaktype', snak['snaktype']),
                        ('datatype', data_type),
                        ('type', c_type)])

# Parser
def parse_file(file):
    with open(file, encoding='utf-8') as data_file:
        with MilleniumDB('graph.txt') as graph:
            for data_line in parse_lines(data_file):
                data = json.loads(data_line)
                node = graph.define_node(data['id'])
                node.add_label(data['type'])
                # last_rev = 'null'
                # modified = 'null'
                if 'lastrevid' in data:
                    # last_rev = data['lastrevid']
                    node.add_property('lastrevid', data['lastrevid'])
                if 'modified' in data:
                    # modified = data['modified']
                    node.add_property('modified', data['modified'])
                node.save()
                label_list = []
                if ('labels' in data) and data['labels']:
                    label_list = data['labels'].values()
                for label in label_list:
                    # TODO: only add english(en) language
                    if label['language'] == "en":
                        a_label = graph.define_anon(
                            labels=['String'],
                            properties=[('string_value', label['value'])])
                        con = node.add_connection(
                            a_label, types=['Label'],
                            properties=[('language', label['language'])])
                        a_label.save()
                        con.save()
                description_list = []
                if ('descriptions' in data) and data['descriptions']:
                    description_list = data['descriptions'].values()
                for description in description_list:
                    # TODO: only add english(en) language
                    if description['language'] == "en":
                        a_desc = graph.define_anon(
                            labels=['String'],
                            properties=[('string_value', description['value'])])
                        con = node.add_connection(
                            a_desc, types=['Description'],
                            properties=[('language', description['language'])])
                        a_desc.save()
                        con.save()
                # alias_list = []
                # if ('aliases' in data) and data['aliases']:
                #     alias_list = reduce(
                #         lambda x, y: x + y, data['aliases'].values())
                # for alias in alias_list:
                #     # TODO: only add english(en) language
                #     if alias['language'] == "en":
                #         a_alias = graph.define_anon(
                #             labels=['String'],
                #             properties=[('string_value', alias['value'])])
                #         con = node.add_connection(
                #             a_alias, types=['Alias'],
                #             properties=[('language', alias['language'])])
                #         a_alias.save()
                #         con.save()

                claim_list = []
                if ('claims' in data) and data['claims']:
                    claim_list = reduce(
                        lambda x, y: x + y, data['claims'].values())
                for claim in claim_list:
                    rank_value = None
                    if 'rank' in claim:
                        rank_value = claim['rank']
                    main_connection = parse_snak(node, claim['mainsnak'],
                                                 graph, rank=rank_value)
                    if ('qualifiers' in claim) and claim['qualifiers']:
                        for qualifier in reduce(
                                lambda x, y: x + y,
                                claim['qualifiers'].values()):
                            if main_connection:
                                parse_snak(main_connection, qualifier, graph,
                                           c_type='Qualifier')
                    # if 'references' in claim:
                    #     for reference in claim['references']:
                    #         for ref_snak in reduce(
                    #                 lambda x, y: x + y,
                    #                 reference['snaks'].values()):
                    #             if main_connection:
                    #                 parse_snak(main_connection, ref_snak,
                    #                            graph, c_type='Reference')
                    if main_connection:
                        main_connection.save()
                # site_list = []
                # if ('sitelinks' in data) and data['sitelinks']:
                #     site_list = data['sitelinks'].values()
                # for site in site_list:
                #     site_props = [('site', site['site']),
                #                   ('title', site['title'])]
                #     site_url = ''
                #     if 'url' in site:
                #         site_url = f', url: {site["url"]}'
                #         site_props.append(('url', site['url']))
                #     a_site = graph.define_anon(
                #         labels=['Sitelink'],
                #         properties=site_props)
                #     con = node.add_connection(a_site, types=['Sitelink'])
                #     a_site.save()
                #     con.save()
                #     for badge in site['badges']:
                #         con = a_site.add_connection(badge, types=['Badge'])
                #         con.save()


if __name__ == '__main__':
    file_path = sys.argv[1]
    parse_file(file_path)
