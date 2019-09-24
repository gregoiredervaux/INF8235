// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <list>
#include "SoftDesignTrainingMainCharacter.h"
#include "CoreMinimal.h"
#include "AIController.h"
#include "SDTAIController.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = AI, config = Game)
class SOFTDESIGNTRAINING_API ASDTAIController : public AAIController
{
    GENERATED_BODY()
public:
    virtual void Tick(float deltaTime) override;
	ASDTAIController ();

	//Rempli deathFloorLocations avec la position de tous les Death Floor
	void FindDeathFloors();

	//Modifie la magnitude du vecteur vitesse de l�agent selon l�acc�l�ration et la vitesse maximale
	FVector GetSpeedVector(FVector speedVector, FVector accVector, float deltaTime);

	//Affiche les axes X,Y,Z relatifs � l'agent
	void DrawCharacterAxes(UWorld * world, APawn * pawn);

	//Retourne vrai si le segment de droite reliant les points start et end traverse un Death Floor
	bool isOnDeathFloor(FVector start, FVector end);

	//Modifie la direction du vecteur vitesse de l�agent selon l��tat et la position du joueur tout en s�occupant d��viter les obstacles
	void HandleCollision(FVector currentLocation);

	//Modifie la direction du vecteur vitesse de l�agent de mani�re � pointer vers un collectible si ce dernier n�est pas bloqu� par un obstacle et se trouve � une distance acceptable
	bool HandleCollect(FVector currentLocation);

	//Retourne le joueur principal
	ASoftDesignTrainingMainCharacter * GetMain();
	
protected:
	float acc; // repr�sente l'acc�l�ration de l'agent
	float vitesseMax; // varaible d�crivant la vitesse maximale pouvant �tre atteinte par l'agent
	FVector speed; // vecteur repr�sentant la vitesse de l'agent selon les 3 axes X,Y,Z
	FVector targetCollectibleLocation; // vecteur contenant la position de l'objet "Collectible" le plus facilement atteignable par l'agent
	float currentRotation; // valeur repr�sentant l'angle de la rotation (selon Yaw) qu'est en train d'effectuer l'agent
	std::list<FVector> deathFloorLocations; // liste de vecteurs repr�sentant les positions des diff�rents pi�ges (ou DeathFloor)
};
