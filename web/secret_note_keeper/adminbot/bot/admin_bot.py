# from app import *
from urllib import parse
import time
import pymysql
from selenium import webdriver
from selenium.common.exceptions import NoSuchElementException

CHALL_URL = "http://challenges.fbctf.com:8082/"
CHALL_URL2 = "http://challenges3.fbctf.com:8082/"

import signal

class timeout:
    def __init__(self, seconds=1, error_message='Timeout'):
        self.seconds = seconds
        self.error_message = error_message
    def handle_timeout(self, signum, frame):
        raise TimeoutError(self.error_message)
    def __enter__(self):
        signal.signal(signal.SIGALRM, self.handle_timeout)
        signal.alarm(self.seconds)
    def __exit__(self, type, value, traceback):
        signal.alarm(0)

def process_url(url, driver):
    try:
        parsed_url = parse.urlparse(url)
        if parsed_url.scheme.startswith("http") is True:
            url = parsed_url.geturl()
            print("url: %s" %(url))
            # Hard timeout
            with timeout(seconds=5):
              driver.get(url)
    except Exception as e:
        print("exception inside process_url")
        print("Exception: ", e)
        return True
    return True

def admin_login(driver):
    try:
        driver.get(CHALL_URL)
        username, password = "admin", "gtrunkgeljndrthuiujgejleuudbcklbeuvbktkgfgcftvkfhkdugrcfheegjjtb"
        username_input = driver.find_element_by_id("username")
        password_input = driver.find_element_by_id("password")
        username_input.send_keys(username)
        password_input.send_keys(password)
        submit = driver.find_element_by_id("submit_button")
        submit.click()
        driver.get(CHALL_URL2) # go to the second URI for tokyo part
        username_input = driver.find_element_by_id("username")
        password_input = driver.find_element_by_id("password")
        username_input.send_keys(username)
        password_input.send_keys(password)
        submit = driver.find_element_by_id("submit_button")
        driver.get(CHALL_URL2) #Go Home
    except Exception as e:
        print("exception inside admin_login")
        print(e)
    return



def get_reports(connection, driver):
    cursor = connection.cursor()
    sql = "SELECT id, link FROM bug WHERE reviewed=0"
    cursor.execute(sql)
    bugs = cursor.fetchall() #TODO not sure if I fetch all .. how participants will affect each other?
    cursor.close()
    reviewed = []
    try:
        for b in bugs:
            #print('bud_id: %d\tbug_owner: %d' % (b.id, b.owner_id))
            print("Bug: " + b['link'])
            if not process_url(b['link'], driver):
              driver = setup_driver()
              admin_login(driver)
            reviewed.append(b['id'])
            print("="*40+"\n")
    finally:
        if len(reviewed) > 0:
            cursor = connection.cursor()
            format_strings = ','.join(['%s'] * len(reviewed))
            sql = "UPDATE bug SET reviewed=1 WHERE id IN (%s);" % format_strings
            print(sql)
            cursor.execute(sql, tuple(reviewed))
            cursor.close()
        connection.commit()

def setup_driver():
    chrome_options = webdriver.ChromeOptions()
    chrome_options.binary_location= "/opt/google/chrome/google-chrome";
    chrome_options.add_argument('--headless')
    chrome_options.add_argument('--disable-gpu')
    driver = webdriver.Chrome(chrome_options=chrome_options)
    driver.implicitly_wait(5)
    driver.set_page_load_timeout(5)
    return driver

import sys

def main():
    # if (len(sys.argv) < 2):
    #   while True:
    #     time.sleep(100)
    time.sleep(2) # just wait for web to get up
    counter = 0
    try:
        while True:
            if counter%10 == 0:
                connection = pymysql.connect(host='web',
                                         user='chal1',
                                         password='getbvllnbunnhriunuglchillki',
                                         db='notes',
                                         charset='utf8mb4',
                                         cursorclass=pymysql.cursors.DictCursor)

                driver = setup_driver()
                admin_login(driver)
            try:
                get_reports(connection, driver)
            except Exception as e:
                print("exception inside main#1")
                print(e)
            if (counter+1)%10 == 0: # we will start the connections in the next round
                #clean up the session after 10 seconds, just for a fresh start
                connection.close()
                driver.quit()
            time.sleep(1)
            counter+=1
    except Exception as e:
        print("exception inside main#2")
        print(e)





if __name__ == '__main__':
    main()
