FROM python:3.6.2
EXPOSE 8080
ENV PYTHONUNBUFFERED 1
RUN mkdir /code
COPY . /code/
WORKDIR /code/src
RUN pip install -r requirements.txt
