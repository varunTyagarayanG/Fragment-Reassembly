#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROWS 100
#define MAX_COLS 5

struct fragment
{
    char ipAddress[16];
    int id;
    int ipAddfragOffSet;
    int moreFragments ;
    int payloadLength;
};

int compare(const void *a, const void *b)
{
    const struct fragment *frag1 = (const struct fragment *)a;
    const struct fragment *frag2 = (const struct fragment *)b;
    int ip_comparison = strcmp(frag1->ipAddress, frag2->ipAddress);
    if (ip_comparison != 0)
        return ip_comparison;
    else
        return frag1->id - frag2->id;
}

void fragmentationAlgorithm(struct fragment fragments[], int count, FILE *output_file)
{
    char current_ip[16] = "";
    int current_id = -1;
    int total_length = 0;
    int expected_offset = 0;
    int mf_count = 0;
    int mf_flag = 0;
    int duplicate_flag = 0;
    int missing_flag = 0;
    int mismatch_flag = 0;

    for (int i = 0; i < count; i++)
    {
        if (strcmp(fragments[i].ipAddress, current_ip) != 0 || fragments[i].id != current_id)
        {
            if (current_id != -1)
            {
                if (mf_count > 1)
                    fprintf(output_file, "%-15s %-5d %s, %s\n", current_ip, current_id, "Missing Fragment", "Duplicate Fragment");
                else if (!mf_flag)
                    fprintf(output_file, "%-15s %-5d %s\n", current_ip, current_id, "Missing Fragment");
                else if (mismatch_flag)
                    fprintf(output_file, "%-15s %-5d %s, %s\n", current_ip, current_id, "Missing Fragment", "Mismatch in length");
                else
                    fprintf(output_file, "%-15s %-5d Complete with Payload = %d\n", current_ip, current_id, total_length);
            }
            strcpy(current_ip, fragments[i].ipAddress);
            current_id = fragments[i].id;
            total_length = 0;
            expected_offset = 0;
            mf_count = 0;
            mf_flag = 0;
            duplicate_flag = 0;
            missing_flag = 0;
            mismatch_flag = 0;
        }

        if (fragments[i].ipAddfragOffSet != expected_offset)
            mismatch_flag = 1;

        if (fragments[i]. moreFragments  == 0)
        {
            mf_count++;
            mf_flag = 1;
        }

        if (mf_count > 1)
            duplicate_flag = 1;

        total_length += fragments[i].payloadLength;
        expected_offset += fragments[i].payloadLength / 8;
    }

    if (current_id != -1)
    {
        if (mf_count > 1)
            fprintf(output_file, "%-15s %-5d %s\n", current_ip, current_id, "Duplicate Fragment");
        else if (!mf_flag)
            fprintf(output_file, "%-15s %-5d %s\n", current_ip, current_id, "Missing Fragment");
        else if (mismatch_flag)
            fprintf(output_file, "%-15s %-5d %s\n", current_ip, current_id, "Mismatch in offset");
        else
            fprintf(output_file, "%-15s %-5d Complete with Payload = %d\n", current_ip, current_id, total_length);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    struct fragment fragments[MAX_ROWS];
    int count = 0;

    FILE *file = fopen(argv[1], "r");
    if (file == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    char header[MAX_COLS][20];
    fscanf(file, "%s %s %s %s %s", header[0], header[1], header[2], header[3], header[4]);

    char ipAddress[16];
    int id, ipAddfragOffSet, moreFragments, payloadLength;
    while (fscanf(file, "%s %d %d %d %d", ipAddress, &id, &ipAddfragOffSet, &moreFragments, &payloadLength) == 5)
    {
        strcpy(fragments[count].ipAddress, ipAddress);
        fragments[count].id = id;
        fragments[count].ipAddfragOffSet = ipAddfragOffSet;
        fragments[count]. moreFragments  = moreFragments;
        fragments[count].payloadLength = payloadLength;
        count++;
    }
    fclose(file);

    qsort(fragments, count, sizeof(struct fragment), compare);

    FILE *output_file = fopen(argv[2], "w");
    if (output_file == NULL)
    {
        perror("Error opening output file");
        return 1;
    }

    fprintf(output_file, "\"Source_IP\" \"Frag_id\" \"Result\"\n");
    fragmentationAlgorithm(fragments, count, output_file);

    fclose(output_file);

    printf("\"Source_IP\" \"Frag_id\" \"Result\"\n");
    printf("--------------------------------------------------------------------------\n");
    printf("%-15s %-5s %s\n", "Source_IP", "Frag_id", "Result");
    printf("--------------------------------------------------------------------------\n");
    fragmentationAlgorithm(fragments, count, stdout);

    return 0;
}
