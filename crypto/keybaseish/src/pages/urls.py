'''
from django.conf.urls import patterns, url
from django.views.generic import TemplateView

urlpatterns = patterns('',
    url('^$', TemplateView.as_view(template_name='index.html')),
)'''

from django.urls import path
from django.views.generic import TemplateView

from . import views

urlpatterns = [
    #path('', views.index, name='index'),
    path('', TemplateView.as_view(template_name='index.html')),
    path('login', TemplateView.as_view(template_name='login.html')),
    path('register', TemplateView.as_view(template_name='register.html')),
    path('download', TemplateView.as_view(template_name='download.html')),
    path('recover', views.recover),
    path('temppass', views.temppass),
    path('messages', views.messages),
]
