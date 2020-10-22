# PYTHON 3
import sys

if len(sys.argv) != 3:
    print("usage: python3 path_to_csv_folder output_filename")
    exit(1)

output = open(sys.argv[2], "w")

def process_node(filename, MASK):
    with open(filename) as f:
        header = True
        column_names = []
        # column_types = []
        for line in f:
            if header:
                header = False
                column_names_and_types = line.split('|')
                # set column names
                for coulumn_name_and_type in column_names_and_types:
                    column_names.append( coulumn_name_and_type.split(':')[0] )
            else:
                columns = line.split('|')

                # Suposing id is the first column and the label is the last column
                # Writing virtual id
                id = int(columns[0])
                max_id = 1 << 56
                if id > max_id:
                    raise Exception("ID too big, MASK won't work")

                output.write("{}".format(id | MASK))
                output.write(" :{}".format(columns[-1].strip().replace(";", " :")))

                output.write(" id:{}".format(id))

                # write properties
                for i in range(1, len(columns)-1):
                    value = columns[i]
                    if len(value) == 0:
                        value = '""'
                    output.write(" {}:{}".format(column_names[i], value))

                output.write("\n")

def process_edge(filename, MASK_FROM, MASK_TO):
    with open(filename) as f:
        header = True
        column_names = []
        for line in f:
            if header:
                header = False
                column_names_and_types = line.split('|')
                # set column names
                for coulumn_name_and_type in column_names_and_types:
                    column_names.append( coulumn_name_and_type.split(':')[0] )
            else:
                columns = line.split('|')

                # Suposing id_from is the first column, id_to is the second and the label is the last

                # Writing virtual id
                id_from = int(columns[0])
                id_to = int(columns[1])
                max_id = 1 << 56
                if id_from > max_id or id_to > max_id:
                    raise Exception("ID too big, MASK won't work")

                output.write("{}->{}".format(id_from | MASK_FROM, id_to | MASK_TO))
                output.write(" :{}".format(columns[-1].strip().replace(";", " :")))

                # write properties
                for i in range(2, len(columns)-1):
                    value = columns[i]
                    if len(value) == 0:
                        value = '""'
                    output.write(" {}:{}".format(column_names[i], value))

                output.write("\n")

process_node(sys.argv[1] + "/organisation_0_0.csv", 1<<56)
process_node(sys.argv[1] + "/place_0_0.csv",        2<<56)
process_node(sys.argv[1] + "/tag_0_0.csv",          3<<56)
process_node(sys.argv[1] + "/tagclass_0_0.csv",     4<<56)
process_node(sys.argv[1] + "/comment_0_0.csv",      5<<56)
process_node(sys.argv[1] + "/forum_0_0.csv",        6<<56)
process_node(sys.argv[1] + "/person_0_0.csv",       7<<56)
process_node(sys.argv[1] + "/post_0_0.csv",         8<<56)

process_edge(sys.argv[1] + "/organisation_isLocatedIn_place_0_0.csv",   1<<56, 2<<56)
process_edge(sys.argv[1] + "/place_isPartOf_place_0_0.csv",             2<<56, 2<<56)
process_edge(sys.argv[1] + "/tag_hasType_tagclass_0_0.csv",             3<<56, 4<<56)
process_edge(sys.argv[1] + "/tagclass_isSubclassOf_tagclass_0_0.csv",   4<<56, 4<<56)
process_edge(sys.argv[1] + "/comment_hasCreator_person_0_0.csv",        5<<56, 7<<56)
process_edge(sys.argv[1] + "/comment_hasTag_tag_0_0.csv",               5<<56, 3<<56)
process_edge(sys.argv[1] + "/comment_isLocatedIn_place_0_0.csv",        5<<56, 2<<56)
process_edge(sys.argv[1] + "/comment_replyOf_comment_0_0.csv",          5<<56, 5<<56)
process_edge(sys.argv[1] + "/comment_replyOf_post_0_0.csv",             5<<56, 8<<56)
process_edge(sys.argv[1] + "/forum_containerOf_post_0_0.csv",           6<<56, 8<<56)
process_edge(sys.argv[1] + "/forum_hasMember_person_0_0.csv",           6<<56, 7<<56)
process_edge(sys.argv[1] + "/forum_hasModerator_person_0_0.csv",        6<<56, 7<<56)
process_edge(sys.argv[1] + "/forum_hasTag_tag_0_0.csv",                 6<<56, 3<<56)
process_edge(sys.argv[1] + "/person_hasInterest_tag_0_0.csv",           7<<56, 3<<56)
process_edge(sys.argv[1] + "/person_isLocatedIn_place_0_0.csv",         7<<56, 2<<56)
process_edge(sys.argv[1] + "/person_knows_person_0_0.csv",              7<<56, 7<<56)
process_edge(sys.argv[1] + "/person_likes_comment_0_0.csv",             7<<56, 5<<56)
process_edge(sys.argv[1] + "/person_likes_post_0_0.csv",                7<<56, 8<<56)
process_edge(sys.argv[1] + "/person_studyAt_organisation_0_0.csv",      7<<56, 1<<56)
process_edge(sys.argv[1] + "/person_workAt_organisation_0_0.csv",       7<<56, 1<<56)
process_edge(sys.argv[1] + "/post_hasCreator_person_0_0.csv",           8<<56, 7<<56)
process_edge(sys.argv[1] + "/post_hasTag_tag_0_0.csv",                  8<<56, 3<<56)
process_edge(sys.argv[1] + "/post_isLocatedIn_place_0_0.csv",           8<<56, 2<<56)

output.close()