/*
 *This program is just for fun ^_^ .
 *
 *Download web pages from "baidu.lecai.com" and analyses it,
 *to get the winning numbers of each phase of the double chromosphere,
 *and count how many times each number appears.
 *
 *author:3null
 *email:yuanyangliu258@gmail.com
 */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netdb.h>
#include<error.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>

#define BASE_URL "http://baidu.lecai.com/lottery/draw/list/50?d="
#define WGET_PROGRAM	"wget"
#define CATCH_FILE_NAME_PREFIX "./.catch_file_"
#define	CORE_FILE_NAME		"./.core_file"
#define SHELL_OUT_TO_NULL	"2> /dev/null"
#define SYSTEM_CMD_BUF_LENGTH_MAX	(1024)
#define	FILE_LINE_LENGTH_MAX			(1024)
#define	FILE_NAME_LENGTH_MAX			(64)

#define	LOTTERY_DATE_LENGTH			(0x0B)
#define	LOTTERY_ISSUE_LENGTH			(0x08)
#define	LOTTERY_BALL_LENGTH			(0x03)
#define	LOTTERY_RED_BALL_NUM			(0x07)

#define LOTTERY_DATE_OFFSET	(0x10)
#define LOTTERY_ISSUE_OFFSET	(0x2E)
#define LOTTERY_BALL_OFFSET	(0x15)

#define BEGIN_TAG	"<tr class=\"bgcolor1\">"
#define LOTTERY_DATE_TAG	"<td class=\"td1\">"
#define LOTTERY_ISSUE_TAG	"<a href=\"/lottery/draw/view/50?phase="
#define LOTTERY_RED_BALL_TAG	"<span class=\"ball_1\">"
#define LOTTERY_BLUE_BALL_TAG	"<span class=\"ball_2\">"

#define	HTTP_PORT				(80)
#define	SOCKET_BUFFER_SIZE	(1024 * 4)

#define	TRUE					(1)
#define	FALSE					(0)
typedef int bool;

char system_cmd[SYSTEM_CMD_BUF_LENGTH_MAX];
char current_file[FILE_NAME_LENGTH_MAX];

typedef struct {
	char lottery_date[LOTTERY_DATE_LENGTH];
	char lottery_issue[LOTTERY_ISSUE_LENGTH];
	char red_ball[LOTTERY_RED_BALL_NUM][LOTTERY_BALL_LENGTH];
	char blue_ball[LOTTERY_BALL_LENGTH];
} lecai_lottery_data_struct;

typedef enum {
	phase_date = 1,
	phase_issue = 2,
	phase_red_ball = 3,
	phase_blue_ball = 4,
} lecai_lottery_find_phase_enum;

const char *host = "baidu.lecai.com";
static char url_suffix[][12] = {
	"2014-01-01",
	"2013-01-01",
	"2012-01-01",
	"2011-01-01",
	"2010-01-01",
	"2009-01-01",
	"2008-01-01",
	"2007-01-01",
	"2006-01-01",
	"2005-01-01",
	"2004-01-01",
	"2003-01-01",
};

static unsigned int total_red_ball[34];
static unsigned int total_blue_ball[17];

int get_page_use_socket(char *url)
{
	struct sockaddr_in servaddr;
	struct hostent *remote_host;
	FILE *catch_file = NULL;
	char buffer[SOCKET_BUFFER_SIZE];
	char send_buffer[512];
	char *p = NULL, year[5];
	int sockfd, len = 0;

	if (strstr(url, BASE_URL)) {
		p = url + strlen(host) + strlen("http://");
	} else {
		exit(1);
	}

	bzero(buffer, sizeof(buffer));
	bzero(&servaddr, sizeof(servaddr));
	bzero(send_buffer, sizeof(send_buffer));

	snprintf(send_buffer, sizeof(send_buffer), "%s%s%s%s%s%s%s%s%s%s",
			 "GET ",
			 p,
			 " HTTP/1.1\r\n",
			 "Host:",
			 host,
			 "\r\n",
			 "Accept: */*\r\n",
			 "User-Agent: Mozilla/4.0(compatible)\r\n",
			 "connection:Keep-Alive\r\n", "\r\n\r\n");

	if ((remote_host = gethostbyname(host)) == 0) {
		printf("Error resolving host: %s\n", host);
		exit(1);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(HTTP_PORT);
	servaddr.sin_addr.s_addr =
		((struct in_addr *)(remote_host->h_addr))->s_addr;

	/* open socket */
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Error opening socket!\n");
		exit(1);
	}

	if (connect(sockfd, (void *)&servaddr, sizeof(servaddr)) == -1) {
		printf("Error connecting to socket\n");
		exit(1);
	}

	if (send(sockfd, send_buffer, strlen(send_buffer), 0) == -1) {
		printf("Error in send\n");
		exit(1);
	}

	strcpy(current_file, CATCH_FILE_NAME_PREFIX);
	p = strchr(url, '=');
	memcpy(year, p + 1, 4);
	strcat(current_file, year);

	do {
		bzero(buffer, sizeof(buffer));
		len = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
		if (len <= 0)
			break;

		*(buffer + len) = '\0';
		catch_file = fopen(current_file, "a+");
		if (catch_file == NULL) {
			printf("Open file %s failed\n", current_file);
			exit(1);
		}

		if (fwrite(buffer, len, 1, catch_file) != 1) {
			printf("Write file[%s] error\n", current_file);
		}

		fclose(catch_file);
	} while (len > 0);

	close(sockfd);

	return 0;
}

int get_page_use_wget(char *url)
{
	char *p = NULL, year[5];

	strcpy(current_file, CATCH_FILE_NAME_PREFIX);
	p = strchr(url, '=');
	memcpy(year, p + 1, 4);
	strcat(current_file, year);

	memset(system_cmd, 0, sizeof(system_cmd));
	snprintf(system_cmd, SYSTEM_CMD_BUF_LENGTH_MAX - 1, "%s %s %s %s %s",
			 WGET_PROGRAM, url, "-O ", current_file, SHELL_OUT_TO_NULL);

	if (0 != system(system_cmd))
		return -1;

	return 0;
}

int page_parse()
{
	FILE *catch_file = NULL, *core_file = NULL;
	char line_text[FILE_LINE_LENGTH_MAX];
	char index = 0, data_valid = 0;
	static char red_bull_num = 0;
	char *find_head = NULL, *find_tag = NULL;

	lecai_lottery_data_struct lottery_data;
	static lecai_lottery_find_phase_enum find_phase = phase_date;

	catch_file = fopen(current_file, "r");
	if (catch_file == NULL) {
		printf("Open file %s failed\n", current_file);
		return -1;
	}

	core_file = fopen(CORE_FILE_NAME, "a");
	if (core_file == NULL) {
		printf("Open file %s failed\n", CORE_FILE_NAME);
		return -1;
	}

	/* 定位到第一次出现" <tr class="bgcolor1"> "的位置 */
	if (NULL == find_head) {
		do {
			(void)fgets(line_text, FILE_LINE_LENGTH_MAX, catch_file);
			find_head = strstr(line_text, BEGIN_TAG);
		} while ((NULL == find_head) && (feof(catch_file) == 0));
	}

	if (find_head != NULL) {
		do {
			find_tag = NULL;
			if (phase_date == find_phase) {
				memset(&lottery_data, 0, sizeof(lottery_data));
			}

			(void)fgets(line_text, FILE_LINE_LENGTH_MAX, catch_file);
			switch (find_phase) {
			case phase_date:
				find_tag = strstr(line_text, LOTTERY_DATE_TAG);
				if (find_tag != NULL) {
					memcpy(lottery_data.lottery_date,
						   find_tag + LOTTERY_DATE_OFFSET, LOTTERY_DATE_LENGTH);
					lottery_data.lottery_date[LOTTERY_DATE_LENGTH - 1] = '\0';
					find_phase = phase_issue;
				}
				break;

			case phase_issue:
				find_tag = strstr(line_text, LOTTERY_ISSUE_TAG);
				if (find_tag != NULL) {
					memcpy(lottery_data.lottery_issue,
						   find_tag + LOTTERY_ISSUE_OFFSET,
						   LOTTERY_ISSUE_LENGTH);
					lottery_data.lottery_issue[LOTTERY_ISSUE_LENGTH - 1] = '\0';
					find_phase = phase_red_ball;
				}
				break;

			case phase_red_ball:
				find_tag = strstr(line_text, LOTTERY_RED_BALL_TAG);
				if (find_tag != NULL) {
					memcpy(lottery_data.red_ball[red_bull_num],
						   find_tag + LOTTERY_BALL_OFFSET, LOTTERY_BALL_LENGTH);
					lottery_data.red_ball[red_bull_num++]
						[LOTTERY_BALL_LENGTH - 1] = '\0';
					if (6 == red_bull_num) {
						find_phase = phase_blue_ball;
						red_bull_num = 0;
					}
				}
				break;

			case phase_blue_ball:
				find_tag = strstr(line_text, LOTTERY_BLUE_BALL_TAG);
				if (find_tag != NULL) {
					memcpy(lottery_data.blue_ball,
						   find_tag + LOTTERY_BALL_OFFSET, LOTTERY_BALL_LENGTH);
					lottery_data.blue_ball[LOTTERY_BALL_LENGTH - 1] = '\0';
					find_phase = phase_date;
					data_valid = 1;
				}
				break;

			default:
				break;
			}

			if (data_valid) {
				data_valid = 0;
				snprintf(line_text, FILE_LINE_LENGTH_MAX,
						 "%s %s %s %s %s %s %s %s %s\n",
						 lottery_data.lottery_date,
						 lottery_data.lottery_issue,
						 lottery_data.red_ball[0],
						 lottery_data.red_ball[1],
						 lottery_data.red_ball[2],
						 lottery_data.red_ball[3],
						 lottery_data.red_ball[4],
						 lottery_data.red_ball[5], lottery_data.blue_ball);

				if (fwrite(line_text, strlen(line_text), 1, core_file) != 1) {
					printf("Write file[%s] error\n", CORE_FILE_NAME);
				}
			}
		} while (feof(catch_file) == 0);
	}

	fclose(catch_file);
	fclose(core_file);

	return 0;
}

int ball_counter(int times)
{
	FILE *core_file = NULL;
	char line_text[FILE_LINE_LENGTH_MAX];
	char index = 0, number = 0;
	lecai_lottery_data_struct lottery_data;
	unsigned int local_times = 0;

	core_file = fopen(CORE_FILE_NAME, "r");
	if (core_file == NULL) {
		printf("Open file %s failed\n", CORE_FILE_NAME);
		return -1;
	}

	do {
		local_times++;
		(void)fgets(line_text, FILE_LINE_LENGTH_MAX, core_file);
		sscanf(line_text, "%11s %8s %3s %3s %3s %3s %3s %3s %3s",
			   lottery_data.lottery_date, lottery_data.lottery_issue,
			   lottery_data.red_ball[0], lottery_data.red_ball[1],
			   lottery_data.red_ball[2], lottery_data.red_ball[3],
			   lottery_data.red_ball[4], lottery_data.red_ball[5],
			   lottery_data.blue_ball);

		for (index = 0; index < 6; index++) {
			number = strtol(lottery_data.red_ball[index], NULL, 10);
			total_red_ball[number]++;
		}

		number = strtol(lottery_data.blue_ball, NULL, 10);
		total_blue_ball[number]++;
	} while ((feof(core_file) == 0) && (local_times <= times));

	fclose(core_file);
	return local_times;
}

void print_result(int times)
{
	int index = 0;

	printf("\nResult as follows:\nRed:\n\t");
	for (index = 1; index < 34; index++) {
		printf("[%-2d]: %-4d ", index, total_red_ball[index]);
		if (index % 5 == 0) {
			printf("\n\t");
		}
	}

	printf("\nBlue\n\t");
	for (index = 1; index < 17; index++) {
		printf("[%-2d]: %-4d ", index, total_blue_ball[index]);
		if (index % 5 == 0) {
			printf("\n\t");
		}
	}
	printf("\n\nTotal [%d] issues.\n\n", times);

	return;
}

void clean(void)
{
	memset(system_cmd, 0, sizeof(system_cmd));
	snprintf(system_cmd, SYSTEM_CMD_BUF_LENGTH_MAX - 1, "%s ",
			 "rm -fr ./.core_file  ./.catch_file_*  2> /dev/null");
	system(system_cmd);
}

int main(int argc, char *argv[])
{
	char url[64];
	char *ptr = NULL, break_tag = 0;
	int issues = 0, index = 0, parse_issues;
	static int issues_one_year = 153, max_issues = 1200;

	if (argc != 2) {
		printf("Usage:\n\t%s [how many issues]\n", argv[0]);
		return 0;
	}

	issues = atoi(argv[1]);
	if (issues <= 0)
		issues = issues_one_year;

	do {
		memset(url, 0, sizeof(url));
		memcpy(url, BASE_URL, strlen(BASE_URL));
		strcat(url, url_suffix[index++]);
		issues -= issues_one_year;

		if (-1 == get_page_use_wget(url)) {
			if (-1 == get_page_use_socket(url)) {
				printf("Get baidu pages failed, the program exit\n");
				exit(1);
			}
		}

		page_parse();

		if (index >= sizeof(url_suffix) / sizeof(url_suffix[0])) {
			break_tag = 1;
			break;
		}
	} while (issues > 0);

	if (break_tag)
		parse_issues =
			issues_one_year * (sizeof(url_suffix) / sizeof(url_suffix[0]));
	else
		parse_issues = issues_one_year * index + issues;

	print_result(ball_counter(parse_issues));
	clean();
	return 0;
}
