<?php
include_once("config.php");
include_once("mysql-func.php");

function todoAddEntry($status,$todo,$openby,$title,$type)
{
  $requete = sprintf("INSERT INTO todo (status,todo,openby,opendate,title,type) VALUES('%s','%s','%s','%s','%s','%s');",$status,$todo,$openby,date("Y-m-d H:i:s"),$title,$type);
  $resultat=exec_requete($requete);
  return $resultat;
}

function todoRemoveEntry($id)
{
  $requete = "DELETE FROM todo WHERE id=".$id.";";
  $resultat=exec_requete($requete);
  return $resultat;
}

function todoModifyEntry($id,$status,$todo,$openby,$closeby,$opendate,$closedate,$title,$type)
{
  $requestDate = date("Y-m-d H:i:s");
  $requete = sprintf("UPDATE todo SET status='%s' , todo='%s' , openby='%s' , closeby='%s' , opendate='%s' , closedate='%s' , title='%s' , type='%s' where id=%s;",$status,$todo,$openby,$closeby,$opendate,$closedate,$title,$type,$id);
  $resultat=exec_requete($requete);
  return $resultat;
}

function todoCloseEntry($id,$closeby,$closedate)
{
  $requestDate = date("Y-m-d H:i:s");
  $requete = sprintf("UPDATE todo SET status='close', closeby='%s', closedate='%s' where id=%s;",$closeby,$closedate,$id);
  $resultat=exec_requete($requete);
  return $resultat;
}

function todoOpenEntry($id,$openby)
{
  $requestDate = date("Y-m-d H:i:s");
  $requete = sprintf("UPDATE todo SET status='open', closeby='%s' where id=%s;",$openby,$id);
  $resultat=exec_requete($requete);
  return $resultat;
}


?>
