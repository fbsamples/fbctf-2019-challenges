# from app import *
from urllib import parse
import time
import pymysql
from selenium import webdriver
from selenium.common.exceptions import NoSuchElementException

def process_url(url, driver):
    try:
        parsed_url = parse.urlparse(url)
        if parsed_url.scheme.startswith("http") is True:
            url = parsed_url.geturl()
            print("url: %s" %(url))
            driver.get(url)
    except Exception as e:
        print("Exception: ", e)
    return True

def admin_login(driver):
    try:
        driver.get("http://web")
        username, password = "admin", "gtrunkgeljndrthuiujgejleuudbcklbeuvbktkgfgcftvkfhkdugrcfheegjjtb"
        username_input = driver.find_element_by_id("username")
        password_input = driver.find_element_by_id("password")
        username_input.send_keys(username)
        password_input.send_keys(password)
        submit = driver.find_element_by_id("submit_button")
        submit.click()
        driver.get("http://web/") # just getting the home page
    except Exception as e:
        print(r)
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
            # print('bud_id: %d\tbug_owner: %d' % (b.id, b.owner_id))
            reviewed.append(b['id'])
            process_url(b['link'], driver)
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
    chrome_options.add_argument('--no-sandbox') #TODO need to cancel this
    chrome_options.add_argument('--headless')
    chrome_options.add_argument('--disable-gpu')
    driver = webdriver.Chrome(chrome_options=chrome_options)
    driver.implicitly_wait(5)
    return driver

def main():
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
                print(e)
            if (counter+1)%10 == 0: # we will start the connections in the next round
                #clean up the session after 10 seconds, just for a fresh start
                connection.close()
                driver.quit()
            time.sleep(1)
            counter+=1
    except Exception as e:
        print(e)





if __name__ == '__main__':
    main()
